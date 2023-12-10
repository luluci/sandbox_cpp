
#include <vector>
#include <tuple>
#include <functional>
#include <iostream>
#include <type_traits>
#include <algorithm>
#include <cstring>
#include <array>
#include <memory>

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

		buffer_interface() : buffer{ 0 }, pos{ 0 }
		{
		}
	};
}

namespace lib_menu {

	enum class event_t {
		None,
		SelectNext,		// 次の項目を選択
		SelectPrev,		// 前の項目を選択
		Confirm,		// 選択している項目で決定する
		Back,			// 上位メニューに戻る

		MAX,
	};

	enum class response_t {
		None,
		OK,				// 操作受理
		NG,				// 操作拒否
		ReqBack,		// 上位メニューに戻る

		MAX
	};

	namespace impl {
		// menu_node_dynにより表示メニュー項目数とmenu_node_*タプル要素数がずれる
		// その差を吸収するための情報構造体
		struct menu_item_info {
			size_t tuple_index;	// メニュー項目の位置と対応するtupleのノード位置をひもづける
			size_t container_index;
			bool is_dyn;
		};

		template <std::size_t N, size_t I, std::size_t... Indices>
		constexpr auto generateArray(std::index_sequence<Indices...>)
		{
			return std::array<menu_item_info, N>{ menu_item_info{ I, Indices, N > 1 }... };
		}

		template <std::size_t N, size_t I>
		constexpr auto generateArray()
		{
			return generateArray<N, I>(std::make_index_sequence<N>{});
		}

		// Helper function to concatenate two arrays
		template <std::size_t... Indices1, std::size_t... Indices2, typename T, std::size_t N1, std::size_t N2>
		constexpr auto concatenateArraysImpl3(const std::array<T, N1>& arr1, const std::array<T, N2>& arr2, std::index_sequence<Indices1...>, std::index_sequence<Indices2...>)
		{
			return std::array<T, N1 + N2>{arr1[Indices1]..., arr2[Indices2]...};
		}

		// Recursive function to concatenate multiple arrays
		template <typename T, size_t TN, typename U, size_t UN>
		constexpr auto concatenateArraysImpl2(const std::array<T, TN>& arr1, const std::array<U, UN>& arr2)
		{
			return concatenateArraysImpl3(arr1, arr2, std::make_index_sequence<TN>(), std::make_index_sequence<UN>());
		}

		// Recursive function to concatenate multiple arrays
		template <typename T>
		constexpr auto concatenateArraysImpl1(const T& arr1)
		{
			return arr1;
		}

		// Recursive function to concatenate multiple arrays
		template <typename T, typename U>
		constexpr auto concatenateArraysImpl1(const T& arr1, const U& arr2)
		{
			return concatenateArraysImpl2(arr1, arr2);
		}

		// Recursive function to concatenate multiple arrays
		template <typename T, typename U, typename... Arrays>
		constexpr auto concatenateArraysImpl1(const T& arr1, const U& arr2, const Arrays &...arrays)
		{
			return concatenateArraysImpl1(concatenateArraysImpl2(arr1, arr2), concatenateArrays(arrays...));
		}

		// Recursive function to concatenate multiple arrays
		template <typename T>
		constexpr auto concatenateArrays(const T& arr)
		{
			return arr;
		}

		// Recursive function to concatenate multiple arrays
		template <typename T, typename... Arrays>
		constexpr auto concatenateArrays(const T& arr, const Arrays &...arrays)
		{
			return concatenateArraysImpl1(arr, concatenateArrays(arrays...));
		}

		template <typename... Args, std::size_t... I>
		constexpr auto gen_array(std::tuple<Args...>& tuple, std::index_sequence<I...>)
		{
			return concatenateArrays(generateArray<std::tuple_element_t<I, std::tuple<Args...>>::ElemSize, I>()...);
		}

		template <typename... Args>
		constexpr auto gen_array(std::tuple<Args...>& tuple)
		{
			return gen_array(tuple, std::make_index_sequence<std::tuple_size<std::tuple<Args...>>::value>());
		}
	}

	namespace impl
	{
		struct tag_menu_node {};
		struct tag_menu_leaf {};
		struct tag_menu_header {};

		auto node_if_hdl_entry = [](auto& item)
			{
				return item.on_entry();
			};
		auto node_if_hdl_exit = [](auto& item)
			{
				item.on_exit();
			};
		auto node_if_hdl_event = [](auto& item, event_t event)
			{
				item.on_event(event);
			};
		auto node_if_hdl_confirm = [](auto& item, size_t index)
			{
				return item.on_confirm(index);
			};
		auto node_if_hdl_back = [](auto& item)
			{
				return item.on_back();
			};
		auto node_if_hdl_render = [](auto& item)
			{
				item.on_render();
			};
		auto node_if_get_label = [](auto& item)
			{
				item.get_label();
			};

		struct node_if
		{
			using children_type = std::vector<node_if>;
			using entry_cb_t = std::function<response_t()>;
			using exit_cb_t = std::function<void()>;
			using event_cb_t = std::function<void(event_t)>;
			using conrifm_cb_t = std::function<response_t(size_t)>;
			using back_cb_t = std::function<void()>;
			using render_cb_t = std::function<void()>;
			using get_label_t = std::function<void()>;

			entry_cb_t on_entry;
			exit_cb_t on_exit;
			event_cb_t on_event;
			conrifm_cb_t on_confirm;
			back_cb_t on_back;
			render_cb_t on_render;
			get_label_t get_label;
		};

		struct data_interface
		{
			// childからparentに渡すデータ
			impl::node_if* next_node;
			impl::node_if::children_type* children_ptr;
			size_t menu_count;
			size_t select_index;
			// parentからchildに渡すデータ
			void* next_dyn_data;
			size_t dyn_index;


			data_interface() : next_node(nullptr), next_dyn_data(nullptr), children_ptr(nullptr), menu_count(0), select_index(0), dyn_index(0)
			{
			}
		};

		template <typename Node>
		auto make_node_if(Node& node)
		{
			return node_if{
				[&]() { return node_if_hdl_entry(node); },
				[&]() { node_if_hdl_exit(node); },
				[&](event_t event) { node_if_hdl_event(node, event); },
				[&](size_t index) { return node_if_hdl_confirm(node, index); },
				[&]() { node_if_hdl_back(node); },
				[&]() { node_if_hdl_render(node); },
				[&]() { node_if_get_label(node); }
			};
		}


		template <typename... Args, std::size_t... I>
		auto tuple_to_vec(std::tuple<Args...>& tuple, std::index_sequence<I...>)
		{
			using Tuple = std::tuple<Args...>;
			return std::vector<node_if>{
				make_node_if(std::get<I>(tuple))...
			};
		}
		template <typename... Args>
		auto tuple_to_vec(std::tuple<Args...>& tuple)
		{
			return tuple_to_vec(tuple, std::make_index_sequence<std::tuple_size<std::tuple<Args...>>::value>());
		}

		template <typename BuffIf, typename DataIf, typename Tuple, std::size_t... I>
		void init_node(BuffIf* buff_if, DataIf* data_if, Tuple& tuple, node_if::children_type& children, std::index_sequence<I...>)
		{
			// 子要素をすべてinit
			auto dummy = {
				(std::get<I>(tuple).init(buff_if, data_if), 0)... };
			// init後にchildを作成
			auto temp = tuple_to_vec(tuple);
			children.swap(temp);
		}
		template <typename BuffIf, typename DataIf, typename Tuple>
		void init_node(BuffIf* buff_if, DataIf* data_if, Tuple& tuple, node_if::children_type& children)
		{
			init_node(buff_if, data_if, tuple, children, std::make_index_sequence<std::tuple_size<Tuple>::value>());
		}

		template <typename... Args, std::size_t... I>
		constexpr size_t get_max_depth(std::tuple<Args...>& tuple, std::index_sequence<I...>)
		{
			return std::max({ std::get<I>(tuple).depth... });
		}
		template <typename... Args>
		constexpr size_t get_max_depth(std::tuple<Args...>& tuple)
		{
			return get_max_depth(tuple, std::make_index_sequence<std::tuple_size<std::tuple<Args...>>::value>());
		}

		// 特定の型がstd::tupleに含まれているかを確認するテンプレート
		template <typename Tuple, std::size_t Index>
		struct tuple_contains_node_header;

		// 基底ケース: タプルが空の場合
		template <std::size_t Index>
		struct tuple_contains_node_header<std::tuple<>, Index>
		{
			static const bool value = false;
		};

		// 再帰ケース: タプルが空でない場合
		template <typename First, typename... Rest, std::size_t Index>
		struct tuple_contains_node_header<std::tuple<First, Rest...>, Index>
		{
			static const bool value = std::is_same<typename First::tag, tag_menu_header>::value || tuple_contains_node_header<std::tuple<Rest...>, Index + 1>::value;
		};

		template <typename Tuple, std::size_t Index>
		inline constexpr bool tuple_contains_node_header_v = tuple_contains_node_header<Tuple, Index>::value;

		// 特定の条件を満たす型を取得するメタプログラム
		template <typename... Types>
		struct get_node_header;

		// 再帰的なメタプログラム
		template <typename First, typename... Rest>
		struct get_node_header<First, Rest...>
		{
			// 条件を満たす場合、型を追加する
			using type = typename std::conditional<std::is_same<typename First::tag, tag_menu_header>::value, // ここに条件を指定
				First,
				typename get_node_header<Rest...>::type>::type;
		};

		// ベースケース: タプルが空の場合
		template <>
		struct get_node_header<>
		{
			using type = void; // 空のタプルを返す
		};

		// 特定の型がstd::tupleに含まれているかを確認するテンプレート
		template <typename Tuple, std::size_t Index>
		struct node_header_pos_in_tuple;

		// 基底ケース: タプルが空の場合
		template <std::size_t Index>
		struct node_header_pos_in_tuple<std::tuple<>, Index>
		{
			static constexpr size_t value = 0;
		};

		// 再帰ケース: タプルが空でない場合
		template <typename First, typename... Rest, std::size_t Index>
		struct node_header_pos_in_tuple<std::tuple<First, Rest...>, Index>
		{
			using type = std::enable_if_t<std::is_same<typename First::tag, tag_menu_header>::value>;
			static constexpr size_t value = Index;
		};

	}

	template <typename Act>
	class menu_header
	{
	public:
		using action_type = Act;
		using tag = impl::tag_menu_header;

	private:
		action_type actor_;

	public:
		menu_header(Act&& actor) : actor_(std::move(actor))
		{
		}

		void on_render(void* data)
		{
			actor_(data);
		}
	};
	template <>
	class menu_header<void>
	{
	public:
		using action_type = void;
		using tag = impl::tag_menu_header;

		menu_header()
		{
		}
		menu_header(menu_header<void>&&)
		{
		}

		void on_render(void*)
		{
		}
	};

	template <typename BuffIf, typename DataIf, typename Node>
	class menu_mng
	{
		using buff_if_type = BuffIf;
		using data_if_type = DataIf;

		//Node node_;
		std::tuple<Node> raw_tuple_;
		Node* root_node_;
		impl::node_if root_node_if_;
		size_t select_idx_;
		int confirm_idx_;
		std::vector<impl::node_if*> confirm_stack_;
		impl::node_if* confirm_ptr_;
		impl::node_if::children_type* children_ptr_;
		size_t children_count_;
		//
		std::unique_ptr<buff_if_type> buff_;
		buff_if_type* buff_ptr_;
		data_if_type data_;

	public:
		size_t depth;

	public:
		menu_mng(Node& node) = delete;

		menu_mng(Node&& node) : raw_tuple_{ std::forward<Node>(node) }, select_idx_(0), confirm_idx_(-1)
		{
			depth = node.depth;
			confirm_stack_.reserve(depth);
		}
		menu_mng(buff_if_type& buff, Node&& node) : menu_mng(std::forward<Node>(node))
		{
			buff_ptr_ = &buff;
		}
		menu_mng(std::unique_ptr<buff_if_type>&& buff, Node&& node) : menu_mng(std::forward<Node>(node))
		{
			buff_ = std::move(buff);
			buff_ptr_ = buff_.get();
		}

		/// @brief 初期化処理を実施
		void init()
		{
			root_node_ = &(std::get<0>(raw_tuple_));
			// tuple内全ノードを初期化
			root_node_->init(buff_ptr_, &data_);
			// rootノードの操作I/Fを作成
			root_node_if_ = impl::make_node_if(*root_node_);
		}

		/// @brief このインスタンスによるメニュー表示処理開始時にコールする。
		/// 初期化後の最初の呼び出し、on_exit()で一度メニュー表示処理を終了した後にコールする想定。
		void on_entry()
		{
			// 初期化
			select_idx_ = 0;
			buff_ptr_->pos = 0;
			confirm_stack_.clear();
			// rootノードを初期状態として登録
			data_.next_node = &root_node_if_;
			data_.next_dyn_data = nullptr;
			auto resp = data_.next_node->on_entry();
			// rootノードに遷移処理
			exec_entry();
			// 表示バッファ更新
			make_disp();
			// メニュー表示
			on_render();
		}

		/// @brief このインスタンスによるメニュー表示処理を終了する時にコールする。
		void on_exit()
		{
			select_idx_ = 0;
			confirm_idx_ = -1;
		}

		// on_event(event)はやめた。
		void on_event(event_t event);

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
			// 表示バッファ更新
			make_disp();
			// メニュー表示処理
			on_render();
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
				// 表示バッファ更新
				make_disp();
				// メニュー表示処理
				on_render();
				break;
			}
		}

		void exec_entry()
		{
			// 遷移先ノード設定
			confirm_stack_.push_back(data_.next_node);
			confirm_ptr_ = data_.next_node;
			select_idx_ = data_.select_index;
			children_ptr_ = data_.children_ptr;
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
			children_ptr_ = data_.children_ptr;
			children_count_ = data_.menu_count;
		}

		void make_disp()
		{
			// 初期化
			select_idx_ = 0;
			buff_ptr_->pos = 0;
			//
			if (children_ptr_ != nullptr)
			{
				// メニュー表示情報作成
				for (auto& node : *children_ptr_)
				{
					node.get_label();
				}
			}
		}
	};

	template <typename BuffIf, typename DataIf, typename Header, typename Tuple>
	struct menu_node
	{
		using tag = impl::tag_menu_node;
		using buff_if_type = BuffIf;
		using data_if_type = DataIf;
		using header_type = menu_header<Header>;
		using children_type = impl::node_if::children_type;
		static constexpr size_t ElemSize = 1;

		char const* label_;
		size_t label_len_;
		buff_if_type* buff_if_;
		data_if_type* data_if_;
		header_type header_;
		Tuple raw_tuple_;
		children_type children_;
		size_t children_count_;
		size_t depth;
		int tgt_;
		std::vector<impl::menu_item_info> index_table_;
		size_t confirm_index_;

		menu_node(char const* label, header_type&& header, Tuple&& tuple)
			: label_(label), header_(std::forward<header_type>(header)), raw_tuple_(std::forward<Tuple>(tuple)), children_count_(0), tgt_(-1)
		{
			label_len_ = std::strlen(label_);
			depth = impl::get_max_depth(raw_tuple_) + 1;
			auto array = impl::gen_array(raw_tuple_);
			index_table_.assign(std::begin(array), std::end(array));
			children_count_ = index_table_.size();
		}

		//menu_node(menu_node& node) = delete;
		//menu_node(menu_node&& node) : label_(node.label_), header_(std::move(node.header_)), raw_tuple_(std::move(node.raw_tuple_)), tgt_(node.tgt_), label_len_(node.label_len_), depth(node.depth), children_(std::move(node.children_)) {
		//}

		void init(buff_if_type* buff_if, data_if_type* data_if)
		{
			buff_if_ = buff_if;
			data_if_ = data_if;

			impl::init_node(buff_if_, data_if_, raw_tuple_, children_);
		}

		response_t on_entry()
		{
			// 自分の情報を通知
			export_info();
			return response_t::None;
		}
		void on_exit()
		{
			confirm_index_ = 0;
		}
		void on_event(event_t event)
		{
		}
		response_t on_confirm(size_t index)
		{
			// 範囲チェックはnode_mngで実施済み
			// childrenが存在するかだけチェックする
			if (children_count_ == 0)
			{
				return response_t::NG;
			}

			// 受理処理を実施
			// 最後に選択したメニューを記憶
			confirm_index_ = index;
			// 遷移先ノードを設定
			auto next_node_info = index_table_[index];
			if (next_node_info.is_dyn) {
				data_if_->dyn_index = next_node_info.container_index;
			}
			data_if_->next_node = &children_[next_node_info.tuple_index];
			data_if_->next_dyn_data = nullptr;

			return response_t::OK;
		}
		void on_back()
		{
			export_info();
		}
		void on_render()
		{
			//
			header_.on_render(nullptr);
		}

		void get_label()
		{
			std::memcpy(buff_if_->buffer[buff_if_->pos], label_, label_len_ + 1);
			buff_if_->pos++;
		}

		void export_info()
		{
			data_if_->children_ptr = &children_;
			data_if_->menu_count = children_count_;
			data_if_->select_index = confirm_index_;
		}
	};

	template <typename T, size_t N, typename BuffIf, typename DataIf, typename Header, typename Tuple>
	struct menu_node_dyn
	{
		using tag = impl::tag_menu_node;
		using data_type = T;
		using buff_if_type = BuffIf;
		using data_if_type = DataIf;
		using header_type = menu_header<Header>;
		using children_type = impl::node_if::children_type;
		static constexpr size_t ElemSize = N;

		char const* label_;
		T(&cont_)[N];
		buff_if_type* buff_if_;
		data_if_type* data_if_;
		header_type header_;
		Tuple raw_tuple_;
		std::vector<impl::node_if> children_;
		size_t children_count_;
		size_t depth;
		int tgt_;
		std::vector<impl::menu_item_info> index_table_;
		size_t confirm_index_;
		//
		size_t cont_index_;

		//menu_node_dyn(menu_node_dyn& node) = delete;
		//menu_node_dyn(menu_node_dyn&& node) : label_(node.label_), cont_(node.cont_), raw_tuple_(std::move(node.raw_tuple_)), children_(std::move(node.children_)), depth(node.depth), tgt_(node.tgt_) {
		//}

		menu_node_dyn(T(&cont)[N], header_type&& header, Tuple&& tuple) : label_(nullptr), cont_(cont), header_(std::forward<header_type>(header)), raw_tuple_(std::forward<Tuple>(tuple)), children_count_(0), tgt_(-1)
		{
			depth = impl::get_max_depth(raw_tuple_) + 1;
			auto array = impl::gen_array(raw_tuple_);
			index_table_.assign(std::begin(array), std::end(array));
			children_count_ = index_table_.size();
		}

		void init(buff_if_type* buff_if, data_if_type* data_if)
		{
			buff_if_ = buff_if;
			data_if_ = data_if;

			impl::init_node(buff_if_, data_if_, raw_tuple_, children_);
		}

		response_t on_entry()
		{
			export_info();
			// import_info
			cont_index_ = data_if_->dyn_index;
			// 選択データチェック
			if (!cont_[cont_index_]) {
				return response_t::NG;
			}
			//
			return response_t::None;
		}
		void on_exit()
		{
			confirm_index_ = 0;
		}
		void on_event(event_t event)
		{
		}
		response_t on_confirm(size_t index)
		{
			// 範囲チェック
			if (index >= N)
			{
				return response_t::NG;
			}
			// childrenが存在するかチェックする
			if (children_count_ == 0)
			{
				return response_t::NG;
			}
			// 選択データをチェック
			if (!(bool)(cont_[index]))
			{
				return response_t::NG;
			}

			// 受理処理を実施
			// 最後に選択したメニューを記憶
			confirm_index_ = index;
			// 遷移先ノードを設定
			auto next_node_info = index_table_[index];
			if (next_node_info.is_dyn) {
				data_if_->dyn_index = next_node_info.container_index;
			}
			data_if_->next_node = &children_[next_node_info.tuple_index];
			data_if_->next_dyn_data = nullptr;

			return response_t::OK;
		}
		void on_back()
		{
			export_info();
		}
		void on_render()
		{
			//
			header_.on_render(nullptr);
		}

		void get_label()
		{
			for (size_t idx = 0; idx < ElemSize; idx++) {
				cont_[idx].get_label(buff_if_->buffer[buff_if_->pos], buff_if_type::BufferColMax);
				buff_if_->pos++;
			}
		}

		void export_info()
		{
			data_if_->children_ptr = &children_;
			data_if_->menu_count = children_count_;
			data_if_->select_index = confirm_index_;
		}
	};

	template <typename BuffIf, typename DataIf, typename Act>
	class menu_leaf
	{
	public:
		using tag = impl::tag_menu_leaf;
		using buff_if_type = BuffIf;
		using data_if_type = DataIf;
		using action_type = Act;
		using children_type = impl::node_if::children_type;
		static constexpr size_t depth = 1;
		static constexpr size_t ElemSize = 1;

	private:
		char const* label_;
		size_t label_len_;
		buff_if_type* buff_if_;
		data_if_type* data_if_;
		action_type actor_;

	public:
		menu_leaf(char const* label, Act& actor) = delete;
		menu_leaf(char const* label, Act&& actor) : label_(label), actor_(std::move(actor))
		{
			label_len_ = std::strlen(label_);
		}

		void init(buff_if_type* buff_if, data_if_type* data_if)
		{
			buff_if_ = buff_if;
			data_if_ = data_if;
		}

		char const* raw_label()
		{
			return label_;
		}

		action_type& get_actor()
		{
			return actor_;
		}
		action_type&& move_actor()
		{
			return std::move(actor_);
		}

		response_t on_entry()
		{
			return actor_.on_entry();
		}
		void on_exit()
		{
		}
		void on_event(event_t event)
		{
		}
		response_t on_confirm(size_t index)
		{
			return actor_.on_confirm(index);
		}
		void on_back()
		{
		}
		void on_render()
		{
		}

		void get_label()
		{
			std::memcpy(buff_if_->buffer[buff_if_->pos], label_, label_len_ + 1);
			buff_if_->pos++;
		}
	};

	template <typename T, typename BuffIf, typename DataIf, typename Act>
	class menu_leaf_dyn
	{
	public:
		using tag = impl::tag_menu_leaf;
		using buff_if_type = BuffIf;
		using data_if_type = DataIf;
		using action_type = Act;
		using data_type = T;
		using children_type = impl::node_if::children_type;
		static constexpr size_t depth = 1;
		static constexpr size_t ElemSize = 1;

	private:
		char const* label_;
		size_t label_len_;
		buff_if_type* buff_if_;
		data_if_type* data_if_;
		action_type actor_;
		data_type* data_;

		using base_leaf_type = menu_leaf<buff_if_type, data_if_type, action_type>;

	public:
		menu_leaf_dyn(base_leaf_type& leaf) = delete;
		menu_leaf_dyn(base_leaf_type&& leaf) : label_(leaf.raw_label()), actor_(leaf.move_actor()), data_(nullptr)
		{
			label_len_ = std::strlen(label_);
		}

		void init(buff_if_type* buff_if, data_if_type* data_if) {
			buff_if_ = buff_if;
			data_if_ = data_if;
		}

		response_t on_entry()
		{
			data_ = (data_type*)data_if_->next_dyn_data;
			data_if_->children_ptr = nullptr;
			return actor_.on_entry();;
		}
		void on_exit()
		{
		}
		void on_event(event_t event)
		{
		}
		response_t on_confirm(size_t index)
		{
			return actor_.on_confirm(index);
		}
		void on_back()
		{
		}
		void on_render()
		{
		}

		void get_label()
		{
			std::memcpy(buff_if_->buffer[buff_if_->pos], label_, label_len_ + 1);
			buff_if_->pos++;
		}
	};

	namespace impl
	{
		template <typename BuffIf, typename DataIf, typename T, typename Tuple, std::size_t I>
		auto make_menu_leaf_dyn(Tuple&& tuple)
		{
			using Elem = typename std::tuple_element<I, Tuple>::type;
			using Act = typename Elem::action_type;
			if constexpr (std::is_same_v<Elem, menu_leaf<BuffIf, DataIf, Act>>) {
				return menu_leaf_dyn<T, BuffIf, DataIf, Act>(std::forward<Elem>(std::get<I>(tuple)));
			}
			else {
				// error
			}
		}

		template <typename BuffIf, typename DataIf, class T, class... Args, std::size_t... I>
		auto make_menu_leaf_dyn(T const& c, std::tuple<Args...>&& tuple, std::index_sequence<I...>)
		{
			return std::make_tuple(
				make_menu_leaf_dyn<BuffIf, DataIf, T, std::tuple<Args...>, I>(std::forward<std::tuple<Args...>>(tuple))...
			);
		}
		template <typename BuffIf, typename DataIf, class T, class... Args>
		auto make_menu_leaf_dyn(T const& c, std::tuple<Args...>&& tuple)
		{
			return make_menu_leaf_dyn<BuffIf, DataIf>(c, std::forward<std::tuple<Args...>>(tuple), std::make_index_sequence<std::tuple_size<std::tuple<Args...>>::value>());
		}
	}

	template <typename Buff = buffer_interface>
	struct builder
	{
		using buff_if_type = Buff;
		using data_if_type = impl::data_interface;
		using buff_inst_type = std::unique_ptr<buff_if_type>;

		template <typename Header, typename... Args>
		static auto make_menu(buff_if_type& buffer, menu_header<Header>&& header, Args &&...nodes)
		{
			auto new_tuple = std::make_tuple(std::forward<Args>(nodes)...);
			auto node_ = menu_node<buff_if_type, data_if_type, Header, decltype(new_tuple)>{ "root", std::forward<menu_header<Header>>(header), std::move(new_tuple) };
			return menu_mng<buff_if_type, data_if_type, decltype(node_)>(buffer, std::move(node_));
		}

		template <typename Header, typename... Args>
		static auto make_menu(menu_header<Header>&& header, Args &&...nodes)
		{
			auto new_tuple = std::make_tuple(std::forward<Args>(nodes)...);
			auto node_ = menu_node<buff_if_type, data_if_type, Header, decltype(new_tuple)>{ "root", std::forward<menu_header<Header>>(header), std::move(new_tuple) };
			return menu_mng<buff_if_type, data_if_type, decltype(node_)>(std::make_unique<buff_if_type>(), std::move(node_));
		}

		template <typename... Args>
		static auto make_menu(Args &&...nodes)
		{
			return make_menu(menu_header<void>(), std::forward<Args>(nodes)...);
		}

		template <typename T, size_t N, typename Header, typename... Args>
		static auto node(T(&c)[N], menu_header<Header>&& header, Args &&...nodes)
		{
			if constexpr (std::is_integral_v<T>)
			{
				// 固定文字列で表示するメニュー
				auto new_tuple = std::make_tuple(std::forward<Args>(nodes)...);
				return menu_node<buff_if_type, data_if_type, Header, decltype(new_tuple)>{c, std::forward<menu_header<Header>>(header), std::move(new_tuple)};
			}
			else
			{
				// コンテナ(T(&c)[N])で動的に表示するメニュー
				auto new_tuple = impl::make_menu_leaf_dyn<buff_if_type, data_if_type>(c[0], std::make_tuple(std::forward<Args>(nodes)...));
				using result_type = menu_node_dyn<T, N, buff_if_type, data_if_type, Header, decltype(new_tuple)>;
				return result_type{ c, std::forward<menu_header<Header>>(header), std::move(new_tuple) };
			}
		}

		template <typename T, size_t N, typename... Args>
		static auto node(T(&c)[N], Args &&...nodes)
		{
			return node(c, menu_header<void>(), std::forward<Args>(nodes)...);
		}

		template <typename Actor>
		static menu_leaf<buff_if_type, data_if_type, Actor> leaf(char const* label, Actor&& actor)
		{
			return menu_leaf<buff_if_type, data_if_type, Actor>{label, std::forward<Actor>(actor)};
		}

		template <typename Actor>
		static menu_header<Actor> header(Actor&& actor)
		{
			return menu_header<Actor>{std::forward<Actor>(actor)};
		}
	};

}

struct data_node
{
	char const* label;
	size_t len;

	static constexpr char charno_data_label[] = "<no data>";
	static constexpr size_t no_data_label_len = sizeof(charno_data_label);

	data_node(): label(nullptr), len(0) {

	}
	data_node(char const* str) : label(str), len(strlen(str)) {

	}

	operator bool()
	{
		return label != nullptr;
	}

	void get_label(char *buffer, size_t buff_size) {
		if (label != nullptr) {
			std::memcpy(buffer, label, std::min(len + 1, buff_size));
		}
		else {
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
	void operator()(void* data)
	{
		std::cout << "<header_root_menu>" << std::endl;
	}
};
struct header_menu_1
{
	void operator()(void* data)
	{
		std::cout << "<header_menu_1>" << std::endl;
	}
};

struct actor_back
{
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

	my_buffer_interface() : buffer{ 0 }, pos{ 0 }
	{
	}
};
my_buffer_interface buffer;

using lm = lib_menu::builder<my_buffer_interface>;

auto menu_back = lm::leaf("[戻る]", actor_back{});

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
);

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

int main()
{
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

	return 0;
}
