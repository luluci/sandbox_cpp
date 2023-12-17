
#include <vector>
#include <tuple>
#include <functional>
#include <iostream>
#include <type_traits>
#include <algorithm>
#include <cstring>
#include <array>
#include <memory>
#include <map>

namespace lib_menu
{
	static constexpr size_t BufferRowMax = 50;
	static constexpr size_t BufferColMax = 255;

	struct buffer_interface
	{
		static constexpr size_t BufferRowMax = BufferRowMax;
		static constexpr size_t BufferColMax = BufferColMax;

		char buffer[BufferRowMax][BufferColMax];
		size_t pos;

		buffer_interface() : buffer{0}, pos{0}
		{
		}
	};
}

namespace lib_menu
{

	enum class event_t
	{
		None,
		SelectNext, // 次の項目を選択
		SelectPrev, // 前の項目を選択
		Confirm,	// 選択している項目で決定する
		Back,		// 上位メニューに戻る

		MAX,
	};

	enum class response_t
	{
		None,
		OK,		   // 操作受理
		NG,		   // 操作拒否
		ReqBack,   // 上位メニューに戻る
		ExitPopup, // ポップアップメニュー終了

		MAX
	};

	namespace impl
	{
		// menu_node_dynにより表示メニュー項目数とmenu_node_*タプル要素数がずれる
		// その差を吸収するための情報構造体
		struct menu_item_info
		{
			size_t tuple_index; // メニュー項目の位置と対応するtupleのノード位置をひもづける
			size_t container_index;
			bool is_dyn;
		};

		template <std::size_t N, size_t I, std::size_t... Indices>
		constexpr auto gen_array(std::index_sequence<Indices...>)
		{
			return std::array<menu_item_info, N>{menu_item_info{I, Indices, N > 1}...};
		}

		template <std::size_t N, size_t I>
		constexpr auto gen_array()
		{
			return gen_array<N, I>(std::make_index_sequence<N>{});
		}

		// Helper function to concatenate two arrays
		template <std::size_t... Indices1, std::size_t... Indices2, typename T, std::size_t N1, std::size_t N2>
		constexpr auto concatenateArraysImpl3(const std::array<T, N1> &arr1, const std::array<T, N2> &arr2, std::index_sequence<Indices1...>, std::index_sequence<Indices2...>)
		{
			return std::array<T, N1 + N2>{arr1[Indices1]..., arr2[Indices2]...};
		}

		// Recursive function to concatenate multiple arrays
		template <typename T, size_t TN, typename U, size_t UN>
		constexpr auto concatenateArraysImpl2(const std::array<T, TN> &arr1, const std::array<U, UN> &arr2)
		{
			return concatenateArraysImpl3(arr1, arr2, std::make_index_sequence<TN>(), std::make_index_sequence<UN>());
		}

		// Recursive function to concatenate multiple arrays
		template <typename T>
		constexpr auto concatenateArraysImpl1(const T &arr1)
		{
			return arr1;
		}

		// Recursive function to concatenate multiple arrays
		template <typename T, typename U>
		constexpr auto concatenateArraysImpl1(const T &arr1, const U &arr2)
		{
			return concatenateArraysImpl2(arr1, arr2);
		}

		// Recursive function to concatenate multiple arrays
		template <typename T, typename U, typename... Arrays>
		constexpr auto concatenateArraysImpl1(const T &arr1, const U &arr2, const Arrays &...arrays)
		{
			return concatenateArraysImpl1(concatenateArraysImpl2(arr1, arr2), concat_arrays(arrays...));
		}

		// Recursive function to concatenate multiple arrays
		template <typename T>
		constexpr auto concat_arrays(const T &arr)
		{
			return arr;
		}

		// Recursive function to concatenate multiple arrays
		template <typename T, typename... Arrays>
		constexpr auto concat_arrays(const T &arr, const Arrays &...arrays)
		{
			return concatenateArraysImpl1(arr, concat_arrays(arrays...));
		}

		template <typename Tuple, std::size_t... I>
		constexpr auto gen_node2index_table_array_impl(std::index_sequence<I...>)
		{
			return concat_arrays(gen_array<std::tuple_element_t<I, Tuple>::label_count, I>()...);
		}

		template <typename Tuple>
		constexpr auto gen_node2index_table_array()
		{
			return gen_node2index_table_array_impl<Tuple>(std::make_index_sequence<std::tuple_size_v<Tuple>>());
		}
	}

	template <typename BuffIf, typename DataIf>
	struct menu_node_base
	{
		using buff_if_type = BuffIf;
		using data_if_type = DataIf;

		using child_type = std::unique_ptr<menu_node_base>;
		using children_type = std::vector<std::unique_ptr<menu_node_base>>;

		children_type children_;
		buff_if_type *buff_if_;
		data_if_type *data_if_;

		menu_node_base()
			: children_(), buff_if_(nullptr), data_if_(nullptr)
		{
		}
		menu_node_base(children_type &&children)
			: children_(std::move(children)), buff_if_(nullptr), data_if_(nullptr)
		{
		}

		void init(buff_if_type *buff_if, data_if_type *data_if)
		{
			buff_if_ = buff_if;
			data_if_ = data_if;

			for (auto &elem : children_)
			{
				elem->init(buff_if, data_if);
			}
		}

		virtual response_t on_entry() = 0;
		virtual void on_exit() = 0;
		virtual response_t on_confirm(size_t index) = 0;
		virtual void on_back() = 0;
		virtual void on_render() = 0;
		virtual void get_label() = 0;
	};

	template <typename Act>
	class menu_header
	{
	public:
		using actor_type = Act;

	private:
		std::unique_ptr<actor_type> actor;
		actor_type *actor_ref;

	public:
		menu_header(std::unique_ptr<actor_type> &&actor_, actor_type *actor_ref_) : actor(std::move(actor_)), actor_ref(actor_ref_)
		{
		}

		void on_render(void *data)
		{
			(*actor_ref)(data);
		}
	};

	template <typename BuffIf, typename DataIf, typename PopupList, size_t Depth>
	class menu_mng
	{
		using self_type = menu_mng<BuffIf, DataIf, PopupList, Depth>;

		using buff_if_type = BuffIf;
		using data_if_type = DataIf;
		using menu_node_type = menu_node_base<BuffIf, DataIf>;
		using child_type = std::unique_ptr<menu_node_type>;
		using children_type = std::vector<std::unique_ptr<menu_node_type>>;
		using popup_menu_list_type = PopupList;
		using popup_menu_key_type = typename PopupList::key_type;

		using confirm_stack_type = std::vector<menu_node_type *>;

		// 所有データ管理
		child_type child_;
		// children_type children_;
		//
		std::unique_ptr<buff_if_type> buff_;
		buff_if_type *buff_ptr_;
		data_if_type data_;

		// メニュー表示管理
		size_t children_count_;
		menu_node_type *confirm_ptr_;
		confirm_stack_type confirm_stack_;
		size_t select_idx_;
		// ポップアップメニュー
		popup_menu_list_type popup_list_;
		size_t popup_bk_buff_ptr_pos_;

	public:
		static constexpr size_t depth = Depth;

	public:
		menu_mng(child_type &&child) : child_(std::move(child)), buff_(), buff_ptr_(nullptr), children_count_(0), confirm_ptr_(), confirm_stack_(), select_idx_(0), popup_list_(), popup_bk_buff_ptr_pos_(0)
		{
			// popupメニュー分+1
			confirm_stack_.reserve(depth + 1);
		}
		menu_mng(buff_if_type &buff, child_type &&child) : menu_mng(std::forward<child_type>(child))
		{
			buff_ptr_ = &buff;
		}
		menu_mng(std::unique_ptr<buff_if_type> &&buff, child_type &&child) : menu_mng(std::forward<child_type>(child))
		{
			buff_ = std::move(buff);
			buff_ptr_ = buff_.get();
		}

		/// @brief 初期化処理を実施
		void init()
		{
			child_->init(buff_ptr_, &data_);
		}

		template <typename T>
		self_type &&add(std::pair<popup_menu_key_type, T> &&value) &&
		{
			popup_list_.insert(std::move(value));

			return std::move(*this);
		}

		/// @brief このインスタンスによるメニュー表示処理開始時にコールする。
		/// 初期化後の最初の呼び出し、on_exit()で一度メニュー表示処理を終了した後にコールする想定。
		response_t on_entry()
		{
			// メニュー選択状況初期化
			select_idx_ = 0;
			buff_ptr_->pos = 0;
			confirm_stack_.clear();
			// rootノードを初期状態として登録
			// 通常のメニュー決定ロジックで実施するように、
			// 通常ロジックをなぞって変数を設定する。
			data_.next_node = child_.get();
			data_.next_dyn_data = nullptr;
			// rootノードにentry処理を実施
			auto resp = data_.next_node->on_entry();
			if (resp != response_t::OK)
			{
				// ここで失敗するのはメニュー構築でミスがある
				return resp;
			}
			// rootノードに遷移処理
			exec_entry();
			// メニュー表示
			on_render();

			return resp;
		}

		/// @brief このインスタンスによるメニュー表示処理を終了する時にコールする。
		void on_exit()
		{
			select_idx_ = 0;
		}

		/// @brief 次のメニュー項目を選択する。
		void on_select_next()
		{
			select_idx_++;
			if (select_idx_ >= children_count_)
			{
				select_idx_ = 0;
			}
			on_render();
		}

		/// @brief 前のメニュー項目を選択する。
		void on_select_prev()
		{
			if (select_idx_ == 0)
			{
				select_idx_ = (int)children_count_ - 1;
			}
			else
			{
				select_idx_--;
			}
			on_render();
		}

		/// @brief 選択中のメニュー項目に対して決定処理をする。
		/// 選択中メニュー項目がnodeなら次のメニュー表示を行い、leafならactionを行う。
		void on_confirm()
		{
			// 現ノードでconfirmイベントを実行
			auto resp = confirm_ptr_->on_confirm(select_idx_);
			switch (resp)
			{
			case response_t::OK:
				check_entry();
				break;

			case response_t::ReqBack:
				on_req_back();
				break;

			case response_t::ExitPopup:
				on_exit_popup();
				break;

			default:
				// その他:何もしない
				break;
			}
		}

		void on_render()
		{
			//
			confirm_ptr_->on_render();
			//
			std::cout << "[menu]" << std::endl;
			size_t idx;
			for (idx = 0; idx < buff_ptr_->pos; idx++)
			{
				if (idx == select_idx_)
				{
					std::cout << "> ";
				}
				else
				{
					std::cout << "  ";
				}

				std::cout << buff_ptr_->buffer[idx] << std::endl;
			}
			std::cout << std::endl;
		}

		void on_req_back()
		{
			// 親メニューに戻る
			exec_exit();
			// メニュー表示処理
			on_render();
		}

		void on_exit_popup()
		{
			// 現ノードから抜ける
			confirm_ptr_->on_exit();
			// 前ノードを選択
			confirm_stack_.pop_back();
			confirm_ptr_ = confirm_stack_.back();
			buff_ptr_->pos = popup_bk_buff_ptr_pos_;
			// メニュー表示処理
			on_render();
		}

		void on_popup(popup_menu_key_type &key)
		{
			// key存在チェック
			if (popup_list_.count(key) == 0)
			{
				return;
			}
			// popupメニュー取得
			auto &popup = popup_list_[key];
			// auto &popup = item.second;
			//
			popup_bk_buff_ptr_pos_ = buff_ptr_->pos;
			buff_ptr_->pos = 0;
			// popupメニュー表示に遷移
			data_.next_node = popup.get();
			data_.select_index = select_idx_;
			check_entry();
		}

	private:
		void check_entry()
		{
			// on_confirmで遷移先のノードが指定されている
			// 次ノードでon_entryをチェック
			// メニュー選択時のアクションはon_entryに記載する
			// アクションは遷移前の現状態を基準に実行する
			auto next_node = data_.next_node;
			auto resp = next_node->on_entry();
			switch (resp)
			{
			case response_t::ReqBack:
				// 前メニューに戻る
				on_req_back();
				break;

			case response_t::NG:
				// entry NG
				break;

			default:
				// その他は遷移確定
				exec_entry();
				// メニュー表示処理
				on_render();
				break;
			}
		}

		void exec_entry()
		{
			//// 遷移先ノード設定
			confirm_stack_.push_back(data_.next_node);
			confirm_ptr_ = data_.next_node;
			select_idx_ = data_.select_index;
			children_count_ = data_.menu_count;
		}

		void exec_exit()
		{
			// 現ノードから抜ける
			confirm_ptr_->on_exit();
			// 前ノードを選択
			confirm_stack_.pop_back();
			confirm_ptr_ = confirm_stack_.back();
			// 前ノードに戻って情報復帰
			confirm_ptr_->on_back();
			select_idx_ = data_.select_index;
			children_count_ = data_.menu_count;
		}
	};

	template <bool IsDyn, typename T, size_t N, size_t Depth, typename BuffIf, typename DataIf, typename Header, typename RawType>
	struct menu_node : menu_node_base<BuffIf, DataIf>
	{
		// ラベルをいくつ持つかの定義:
		//   labelを持つデータ型のN個の配列
		//   or ラベル文字列を1個
		using label_type = T;
		static constexpr size_t label_count = N;

		using buff_if_type = BuffIf;
		using data_if_type = DataIf;
		using base_type = menu_node_base<BuffIf, DataIf>;
		using header_type = menu_header<Header>;
		static constexpr size_t depth = Depth;
		static constexpr bool IsDynamic = IsDyn;

		using child_type = typename base_type::child_type;
		using children_type = typename base_type::children_type;

		// temp_menu_raw_type
		using raw_type = RawType;

		// 固定ラベル表示ノードデータ
		char const *label_;
		size_t label_len_;
		// 配列表示ノードデータ
		T(*cont_)
		[N];

		// 共通データ
		header_type header_;

		size_t children_count_;
		int tgt_;
		std::vector<impl::menu_item_info> index_table_;
		size_t confirm_index_;
		//
		size_t cont_index_;

		menu_node(char const *label, header_type &&header, children_type &&children)
			: label_(label), label_len_(0), cont_(nullptr), header_(std::forward<header_type>(header)), children_count_(0), confirm_index_(0), tgt_(-1), base_type(std::move(children))
		{
			static_assert(!IsDynamic, "dynノードでないときは固定ラベルを指定する");

			label_len_ = std::strlen(label_);
			init_impl();
		}

		menu_node(T (&cont)[N], header_type &&header, children_type &&children)
			: label_(nullptr), label_len_(0), cont_(&cont), header_(std::forward<header_type>(header)), children_count_(0), confirm_index_(0), tgt_(-1), base_type(std::move(children))
		{
			static_assert(IsDynamic, "dynノードのときはラベルコンテナを指定する");

			init_impl();
		}

		void init_impl()
		{
			// dynメニューではchildの数と表示メニュー項目の数が一致しない
			// 選択メニュー番号からchildのインデックスへ変換するテーブルを生成しておく
			auto &array = raw_type::node2index_table;
			index_table_.assign(std::begin(array), std::end(array));
			children_count_ = index_table_.size();
		}

		response_t on_entry()
		{
			// 自分の情報を通知
			export_info();

			if constexpr (IsDynamic)
			{
				// 上位からの情報を取り込む
				cont_index_ = this->data_if_->dyn_index;
				// 選択データチェック
				if (!(*cont_)[cont_index_])
				{
					return response_t::NG;
				}
			}
			//
			make_disp();
			//
			return response_t::OK;
		}
		void on_exit()
		{
			confirm_index_ = 0;
		}

		response_t on_confirm(size_t index)
		{
			// 範囲チェックはnode_mngで実施済み
			// childrenが存在するかチェックする
			if (children_count_ == 0)
			{
				return response_t::NG;
			}

			// 受理処理を実施
			// 最後に選択したメニューを記憶
			confirm_index_ = index;
			// 遷移先ノードを設定
			auto next_node_info = index_table_[index];
			this->data_if_->next_node = (this->children_[next_node_info.tuple_index]).get();
			if (next_node_info.is_dyn)
			{
				this->data_if_->dyn_index = next_node_info.container_index;
				this->data_if_->next_dyn_data = (void *)&((*cont_)[this->data_if_->dyn_index]);
			}
			else
			{
				this->data_if_->next_dyn_data = nullptr;
			}

			return response_t::OK;
		}
		void on_back()
		{
			export_info();
			make_disp();
		}
		void on_render()
		{
			//
			header_.on_render(nullptr);
		}

		void make_disp()
		{
			// 初期化
			this->buff_if_->pos = 0;
			// メニュー表示情報作成
			for (auto &node : this->children_)
			{
				node->get_label();
			}
		}

		void get_label()
		{
			if constexpr (IsDynamic)
			{
				for (size_t idx = 0; idx < label_count; idx++)
				{
					(*cont_)[idx].get_label(this->buff_if_->buffer[this->buff_if_->pos], buff_if_type::BufferColMax);
					this->buff_if_->pos++;
				}
			}
			else
			{
				std::memcpy(this->buff_if_->buffer[this->buff_if_->pos], label_, label_len_ + 1);
				this->buff_if_->pos++;
			}
		}

		void export_info()
		{
			this->data_if_->menu_count = children_count_;
			this->data_if_->select_index = confirm_index_;
		}
	};

	template <bool IsDyn, typename DynData, typename BuffIf, typename DataIf, typename Act>
	class menu_leaf : menu_node_base<BuffIf, DataIf>
	{
	public:
		using label_type = char;
		static constexpr size_t label_count = 1;

		using base_type = menu_node_base<BuffIf, DataIf>;
		using buff_if_type = BuffIf;
		using data_if_type = DataIf;
		using actor_type = Act;
		using data_type = DynData;
		static constexpr size_t depth = 1;
		static constexpr bool IsDynamic = IsDyn;

	private:
		char const *label_;
		size_t label_len_;
		// actorの所有権を持つ場合はunique_ptrで保持する
		// 参照を受け取った場合は参照用ポインタのみ保持する
		std::unique_ptr<actor_type> actor;
		actor_type *actor_ref;
		// dynメニュー用定義
		// 上位階層でコンテナからデータを選択する場合、
		// 選択したデータへの参照を保持する
		data_type *data_;

	public:
		menu_leaf(char const *label, std::unique_ptr<actor_type> &&actor_, actor_type *actor_ref_)
			: label_(label), actor(std::move(actor_)), actor_ref(actor_ref_), base_type()
		{
			label_len_ = std::strlen(label_);
		}

		response_t on_entry()
		{
			if constexpr (IsDynamic)
			{
				data_ = (data_type *)this->data_if_->next_dyn_data;
			}
			return actor_ref->on_entry();
		}
		void on_exit()
		{
		}
		void on_event(event_t event)
		{
		}
		response_t on_confirm(size_t index)
		{
			return actor_ref->on_confirm(index);
		}
		void on_back()
		{
		}
		void on_render()
		{
		}

		void get_label()
		{
			std::memcpy(this->buff_if_->buffer[this->buff_if_->pos], label_, label_len_ + 1);
			this->buff_if_->pos++;
		}

		char const *raw_label()
		{
			return label_;
		}

		actor_type &get_actor()
		{
			return actor;
		}
		actor_type &&move_actor()
		{
			return std::move(actor);
		}
	};

	template <typename Key, typename BuffIf, typename DataIf, typename Act>
	class menu_popup : menu_node_base<BuffIf, DataIf>
	{
	public:
		using key_type = Key;

		using label_type = char;
		static constexpr size_t label_count = 0;

		using base_type = menu_node_base<BuffIf, DataIf>;
		using buff_if_type = BuffIf;
		using data_if_type = DataIf;
		using actor_type = Act;
		using data_type = void;
		static constexpr size_t depth = 1;
		static constexpr bool IsDynamic = false;

	private:
		// actorの所有権を持つ場合はunique_ptrで保持する
		// 参照を受け取った場合は参照用ポインタのみ保持する
		std::unique_ptr<actor_type> actor;
		actor_type *actor_ref;

	public:
		menu_popup(std::unique_ptr<actor_type> &&actor_, actor_type *actor_ref_)
			: actor(std::move(actor_)), actor_ref(actor_ref_), base_type()
		{
		}
		menu_popup(actor_type &actor_)
			: actor(), actor_ref(&actor_)
		{
		}
		menu_popup(actor_type &&actor_)
			: actor(std::make_unique<actor_type>(std::move(actor_))), actor_ref(actor.get())
		{
		}

		response_t on_entry()
		{
			return actor_ref->on_entry();
		}
		void on_exit()
		{
		}
		void on_event(event_t event)
		{
		}
		response_t on_confirm(size_t index)
		{
			return actor_ref->on_confirm(index);
		}
		void on_back()
		{
		}
		void on_render()
		{
			actor_ref->on_render();
		}

		void get_label()
		{
		}

		char const *raw_label()
		{
			return "";
		}

		actor_type &get_actor()
		{
			return actor;
		}
		actor_type &&move_actor()
		{
			return std::move(actor);
		}
	};

	template <typename Buff = buffer_interface, typename PopupKey = size_t>
	class builder
	{
		struct data_interface;

		using buff_if_type = Buff;
		using data_if_type = data_interface;
		using buff_inst_type = std::unique_ptr<buff_if_type>;
		using base_type = menu_node_base<buff_if_type, data_if_type>;
		// menu要素定義
		using child_type = typename base_type::child_type;
		using children_type = typename base_type::children_type;
		// popup menu要素定義
		using popup_menu_key_type = PopupKey;
		using popup_menu_list_type = std::map<popup_menu_key_type, child_type>;

		struct data_interface
		{
			// childからparentに渡すデータ
			base_type *next_node;
			size_t menu_count;
			size_t select_index;
			// parentからchildに渡すデータ
			void *next_dyn_data;
			size_t dyn_index;

			data_interface() : next_node(nullptr), menu_count(0), select_index(0), next_dyn_data(nullptr), dyn_index(0)
			{
			}
		};

		template <bool IsDyn, typename T, size_t N, size_t Depth, typename HeaderActor, typename RawChildren>
		struct temp_menu_node
		{
			// ラベルをいくつ持つかの定義:
			//   labelを持つデータ型のN個の配列
			//   or ラベル文字列を1個
			using label_type = T;
			static constexpr size_t label_count = N;

			static constexpr size_t depth = Depth;
			using header_actor_type = HeaderActor;
			using child_raw_type_tuple = RawChildren;
			static constexpr bool IsDynamic = IsDyn;

			temp_menu_node() {}

			// label_type(&label_ref)[label_len];
			// header_type&& header;
			// children_type&& children;
			// temp_menu_node(T(&label_)[N], header_type&& header_, children_type&& children_)
			//	: label_ref(label_), header(std::forward<header_type>(header_)), children(std::forward<children_type>(children_))
			//{}
			// temp_menu_node(temp_menu_node&) = delete;
			// temp_menu_node(temp_menu_node&& node)
			//	: temp_menu_node(node.label_ref, std::move(node.header), std::move(node.children))
			//{}
		};
		template <typename Actor>
		struct temp_menu_leaf
		{
			using label_type = char;
			static constexpr size_t label_count = 1;

			using actor_type = Actor;
			static constexpr size_t depth = 1;

			char const *label_ref;
			std::unique_ptr<actor_type> actor;
			actor_type *actor_ref;

			temp_menu_leaf(char const *label_, Actor &actor_)
				: label_ref(label_), actor(), actor_ref(&actor_)
			{
			}
			temp_menu_leaf(char const *label_, Actor &&actor_)
				: label_ref(label_), actor(std::make_unique<actor_type>(std::move(actor_))), actor_ref(actor.get())
			{
			}
		};
		template <typename Actor>
		struct temp_menu_header
		{
			using actor_type = Actor;

			std::unique_ptr<actor_type> actor;
			actor_type *actor_ref;

			temp_menu_header() : actor(), actor_ref(nullptr) {}
			temp_menu_header(Actor &actor_)
				: actor(), actor_ref(&actor_)
			{
			}
			temp_menu_header(Actor &&actor_)
				: actor(std::make_unique<actor_type>(std::move(actor_))), actor_ref(actor.get())
			{
			}
		};
		struct temp_menu_header_dummy
		{
			void operator()(void *data)
			{
			}
		};

		struct tag_temp_menu_node
		{
		};
		struct tag_temp_menu_leaf
		{
		};
		template <typename Tag, typename Node>
		struct temp_menu_holder
		{
			using tag = Tag;
			using node_type = Node;
			node_type node;
			child_type child;

			/*
			temp_menu_holder(node_type&& node_, child_type&& child_)
				: node(std::forward<node_type>(node_)), child(std::move(child))
			{

			}
			temp_menu_holder(temp_menu_holder&& holder)
				: node(std::move(holder.node)), child(std::move(holder.child))
			{

			}
			*/
		};

		template <typename T>
		struct elem_info
		{
			static constexpr size_t depth = std::remove_reference_t<T>::depth;
		};

		template <typename... Args>
		struct temp_menu_raw_type
		{
			using tuple_type = std::tuple<typename std::remove_reference<Args>::type::node_type...>;
			static constexpr size_t depth = std::max({elem_info<typename std::remove_reference<Args>::type::node_type>::depth...});
			static constexpr auto node2index_table = impl::gen_node2index_table_array<tuple_type>();
		};

		template <typename Actor>
		static auto make_child_nml_impl(temp_menu_leaf<Actor> &leaf)
		{
			return child_type{
				(base_type *)new menu_leaf<false, void, buff_if_type, data_if_type, Actor>{
					leaf.label_ref, std::move(leaf.actor), leaf.actor_ref}};
		}

		template <typename Node>
		static auto make_child_nml(Node &&node)
		{
			if constexpr (std::is_same_v<typename std::remove_reference<Node>::type::tag, tag_temp_menu_leaf>)
			{
				return make_child_nml_impl(std::forward<Node>(node).node);
			}
			else
			{
				return std::move(node.child);
			}
		}

		template <typename T, typename Actor>
		static auto make_child_dyn_impl(temp_menu_leaf<Actor> &leaf)
		{
			return child_type{
				(base_type *)new menu_leaf<true, T, buff_if_type, data_if_type, Actor>{
					leaf.label_ref, std::move(leaf.actor), leaf.actor_ref}};
		}

		template <typename T, typename Node>
		static auto make_child_dyn(Node &&node)
		{
			if constexpr (std::is_same_v<typename std::remove_reference<Node>::type::tag, tag_temp_menu_leaf>)
			{
				return make_child_dyn_impl<T>(std::forward<Node>(node).node);
			}
			else
			{
				// error
			}
		}

		template <typename Vec, typename T>
		static void make_children_nml_vec(Vec &vec, T &&node)
		{
			vec.push_back(make_child_nml(std::forward<T>(node)));
		}
		template <typename Vec, typename T, typename... Args>
		static void make_children_nml_vec(Vec &vec, T &&node, Args &&...nodes)
		{
			vec.push_back(make_child_nml(std::forward<T>(node)));
			make_children_nml_vec(vec, std::forward<Args>(nodes)...);
		}

		template <typename... Args>
		static auto make_children_nml(Args &&...nodes)
		{
			children_type children;
			make_children_nml_vec(children, std::forward<Args>(nodes)...);
			return std::move(children);
		}

		template <typename Data, typename Vec, typename T>
		static void make_children_dyn_vec(Vec &vec, T &&node)
		{
			vec.push_back(make_child_dyn<Data>(std::forward<T>(node)));
		}
		template <typename Data, typename Vec, typename T, typename... Args>
		static void make_children_dyn_vec(Vec &vec, T &&node, Args &&...nodes)
		{
			vec.push_back(make_child_dyn<Data>(std::forward<T>(node)));
			make_children_nml_vec(vec, std::forward<Args>(nodes)...);
		}

		template <typename T, typename... Args>
		static auto make_children_dyn(Args &&...nodes)
		{
			children_type children;
			make_children_dyn_vec<T>(children, std::forward<Args>(nodes)...);
			return std::move(children);
		}

		template <typename T, size_t N, typename Header, typename... Args>
		static auto make_node_nml_impl(T (&c)[N], temp_menu_header<Header> &&header, Args &&...nodes)
		{
			auto children = make_children_nml(nodes...);
			auto new_header = menu_header<Header>(std::move(header.actor), header.actor_ref);
			using raw_type = temp_menu_raw_type<Args...>;
			using child_raw_type_tuple = typename raw_type::tuple_type;
			using raw_node_type = temp_menu_node<false, int, 1, raw_type::depth + 1, Header, raw_type>;
			using node_type = menu_node<false, int, 1, raw_type::depth + 1, buff_if_type, data_if_type, Header, raw_type>;

			return temp_menu_holder<tag_temp_menu_node, raw_node_type>{
				raw_node_type{},
				child_type{(base_type *)new node_type{c, std::move(new_header), std::move(children)}}};
		}

		template <typename T, size_t N, typename Header, typename... Args>
		static auto make_node_dyn_impl(T (&c)[N], temp_menu_header<Header> &&header, Args &&...nodes)
		{
			auto children = make_children_dyn<T>(nodes...);
			auto new_header = menu_header<Header>(std::move(header.actor), header.actor_ref);

			using raw_type = temp_menu_raw_type<Args...>;
			using child_raw_type_tuple = typename raw_type::tuple_type;
			using raw_node_type = temp_menu_node<true, T, N, raw_type::depth + 1, Header, child_raw_type_tuple>;
			using node_type = menu_node<true, T, N, raw_type::depth + 1, buff_if_type, data_if_type, Header, raw_type>;

			return temp_menu_holder<tag_temp_menu_node, raw_node_type>{
				raw_node_type{},
				child_type{(base_type *)new node_type{c, std::move(new_header), std::move(children)}}};
		}

		template <typename T, size_t N, typename Header, typename... Args>
		static auto make_node_impl(T (&c)[N], temp_menu_header<Header> &&header, Args &&...nodes)
		{
			if constexpr (std::is_integral_v<T>)
			{
				// 固定文字列で表示するメニュー
				return make_node_nml_impl(c, std::forward<temp_menu_header<Header>>(header), std::forward<Args>(nodes)...);
			}
			else
			{
				// コンテナ(T(&c)[N])で動的に表示するメニュー
				return make_node_dyn_impl(c, std::forward<temp_menu_header<Header>>(header), std::forward<Args>(nodes)...);
			}
		}

	public:
		template <typename Header, typename... Args>
		static auto make_menu(buff_if_type &buffer, Header &&header, Args &&...nodes)
		{
			auto root = make_node_impl("root", std::forward<Header>(header), std::forward<Args>(nodes)...);
			using root_type = decltype(root);
			using raw_node_type = typename root_type::node_type;

			return menu_mng<buff_if_type, data_if_type, popup_menu_list_type, raw_node_type::depth>(buffer, std::move(root.child));
		}

		template <typename Header, typename... Args>
		static auto make_menu(Header &&header, Args &&...nodes)
		{
			auto root = make_node_impl("root", std::forward<Header>(header), std::forward<Args>(nodes)...);
			using root_type = decltype(root);
			using raw_node_type = typename root_type::node_type;

			return menu_mng<buff_if_type, data_if_type, popup_menu_list_type, raw_node_type::depth>(std::make_unique<buff_if_type>(), std::move(root.child));
		}

		template <typename... Args>
		static auto make_menu(Args &&...nodes)
		{
			return make_menu(temp_menu_header<temp_menu_header_dummy>(), std::forward<Args>(nodes)...);
		}

		template <typename T, size_t N, typename Header, typename... Args>
		static auto node(T (&c)[N], temp_menu_header<Header> &&header, Args &&...nodes)
		{
			return make_node_impl(c, std::forward<temp_menu_header<Header>>(header), std::forward<Args>(nodes)...);
		}

		template <typename T, size_t N, typename... Args>
		static auto node(T (&c)[N], Args &&...nodes)
		{
			return make_node_impl(c, temp_menu_header<temp_menu_header_dummy>(), std::forward<Args>(nodes)...);
		}

		template <typename Actor>
		static auto leaf(char const *label, Actor &&actor)
		{
			using node_type = temp_menu_leaf<Actor>;
			return temp_menu_holder<tag_temp_menu_leaf, node_type>{
				node_type{label, std::forward<Actor>(actor)},
				child_type(),
			};
		}

		template <typename Actor>
		static auto header(Actor &&actor)
		{
			return temp_menu_header<Actor>{std::forward<Actor>(actor)};
		}

		template <typename Actor>
		static auto popup(popup_menu_key_type &&key, Actor &&actor)
		{
			using node_type = menu_popup<popup_menu_key_type, buff_if_type, data_if_type, Actor>;
			auto popup = child_type((base_type *)new node_type(std::forward<Actor>(actor)));
			return std::make_pair(std::move(key), std::move(popup));
		}
	};

}

namespace my_menu
{
	enum class popup_menu
	{
		progress,
		alert,
	};

	struct popup_progress
	{
		lib_menu::response_t on_entry()
		{
			return lib_menu::response_t::OK;
		}
		lib_menu::response_t on_confirm(size_t index)
		{
			return lib_menu::response_t::ExitPopup;
		}
		void on_render()
		{
			std::cout << "popup_progress" << std::endl;
		}
	};

	struct data_node
	{
		char const *label;
		size_t len;

		static constexpr char charno_data_label[] = "<no data>";
		static constexpr size_t no_data_label_len = sizeof(charno_data_label);

		data_node() : label(nullptr), len(0)
		{
		}
		data_node(char const *str) : label(str), len(strlen(str))
		{
		}

		operator bool()
		{
			return label != nullptr;
		}

		void get_label(char *buffer, size_t buff_size)
		{
			if (label != nullptr)
			{
				std::memcpy(buffer, label, std::min(len + 1, buff_size));
			}
			else
			{
				std::memcpy(buffer, charno_data_label, std::min(no_data_label_len, buff_size));
			}
		}
	};

	data_node data_c[] = {
		data_node{"item1"},
		data_node{},
		data_node{"item3"},
	};

	struct header_root_menu
	{
		void operator()(void *data)
		{
			std::cout << "<header_root_menu>" << std::endl;
		}
	};
	struct header_menu_1
	{
		void operator()(void *data)
		{
			std::cout << "<header_menu_1>" << std::endl;
		}
	};

	struct actor_back
	{
		/*
		actor_back(){
			// ctor
		}
		actor_back(actor_back const& obj){
			std::cout << "copy ctor" << std::endl;
		}
		actor_back(actor_back &&obj)
		{
			std::cout << "move ctor" << std::endl;
		}
		*/

		lib_menu::response_t on_entry()
		{
			return lib_menu::response_t::ReqBack;
		}
		lib_menu::response_t on_confirm(size_t index)
		{
			return lib_menu::response_t::ReqBack;
		}
	};

	struct my_buffer_interface
	{
		static constexpr size_t BufferRowMax = lib_menu::BufferRowMax;
		static constexpr size_t BufferColMax = lib_menu::BufferColMax;

		char buffer[lib_menu::BufferRowMax][lib_menu::BufferColMax];
		size_t pos;

		my_buffer_interface() : buffer{0}, pos{0}
		{
		}
	};
	my_buffer_interface buffer;

	using lm = lib_menu::builder<my_buffer_interface, popup_menu>;

	auto menu_back = lm::leaf("[戻る]", actor_back{});

	// clang-format off
	auto menu_mng = lm::make_menu(
		buffer,
		lm::header(header_root_menu()),

		lm::node("menu_1",
			lm::header(header_menu_1()),

			lm::node(data_c,
				lm::leaf("dyn_menu_1", actor_back{}),
				lm::leaf("dyn_menu_2", actor_back{}),
				lm::leaf("dyn_menu_3", actor_back{}),
				menu_back
			),
			lm::leaf("menu_1_2", actor_back{}),
			menu_back
		),

		lm::node("menu_2",
			lm::leaf("menu_2_1", actor_back{}),
			lm::leaf("menu_2_2", actor_back{}),
			lm::leaf("menu_2_3", actor_back{}),
			menu_back
		),

		lm::node("menu_3",
			lm::node("menu_3_1",
				lm::leaf("menu_3_1_1", actor_back{}),
				lm::node("menu_3_1_2",
					lm::leaf("menu_3_1_2_1", actor_back{}),
					lm::leaf("menu_3_1_2_2", actor_back{}),
					lm::leaf("menu_3_1_2_3", actor_back{}),
					menu_back
				),
				lm::leaf("menu_3_1_3", actor_back{}),
				menu_back
			),
			lm::leaf("menu_3_2", actor_back{}),
			menu_back
		),

		lm::leaf("test", actor_back{}),
		lm::leaf("Version", actor_back{})
	).add(lm::popup(popup_menu::progress, popup_progress{}));
	// clang-format on

	void event_next()
	{
		std::cout << "[event:[SelectNext]]" << std::endl;
		menu_mng.on_select_next();
	}
	void event_prev()
	{
		std::cout << "[event:[SelectPrev]]" << std::endl;
		menu_mng.on_select_prev();
	}
	void event_confirm()
	{
		std::cout << "[event:[Confirm]]" << std::endl;
		menu_mng.on_confirm();
	}
	void event_popup(popup_menu tgt)
	{
		menu_mng.on_popup(tgt);
	}
}

int main()
{
	using namespace my_menu;

	menu_mng.init();
	menu_mng.on_entry();
	event_next();
	event_next();
	event_next();
	event_next();
	event_next();
	event_prev();
	event_next();
	event_next();
	event_confirm();
	event_next();
	event_next();
	event_next();
	event_confirm();
	event_prev();
	event_confirm();
	event_next();
	event_next();
	event_next();
	event_next();
	event_next();
	event_next();
	event_confirm();
	event_next();
	event_confirm();
	event_next();
	event_next();
	event_next();
	event_next();
	event_next();
	event_popup(popup_menu::progress);
	event_confirm();

	return 0;
}
