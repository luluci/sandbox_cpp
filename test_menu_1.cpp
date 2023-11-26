
#include <vector>
#include <tuple>
#include <functional>
#include <iostream>
#include <type_traits>
#include <algorithm>
#include <cstring>
#include <array>

namespace lib_menu {
	static constexpr size_t BufferRowMax = 50;
	static constexpr size_t BufferColMax = 255;

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
		template <std::size_t N, size_t I, std::size_t... Indices>
		constexpr auto generateArray(std::index_sequence<Indices...>)
		{
			return std::array<size_t, N>{((void)Indices, I)...};
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
		template <typename T, typename U>
		constexpr auto concatenateArraysImpl1(const T& arr1, const U& arr2)
		{
			return concatenateArraysImpl2(arr1, arr2);
		}

		// Recursive function to concatenate multiple arrays
		template <typename T, typename U, typename... Arrays>
		constexpr auto concatenateArraysImpl1(const T& arr1, const U& arr2, const Arrays &...arrays)
		{
			return concatenateArraysImpl1(concatenateArraysImpl3(arr1, arr2), concatenateArrays(arrays...));
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

		struct DataInterface;

		auto node_if_hdl_entry = [](auto& item, DataInterface& data)
			{
				return item.on_entry(data);
			};
		auto node_if_hdl_exit = [](auto& item)
			{
				item.on_exit();
			};
		auto node_if_hdl_event = [](auto& item, event_t event)
			{
				item.on_event(event);
			};
		auto node_if_hdl_confirm = [](auto& item, DataInterface& data, size_t index)
			{
				return item.on_confirm(data, index);
			};
		auto node_if_hdl_render = [](auto& item)
			{
				item.on_render();
			};
		auto node_if_get_label = [](auto& item, DataInterface& data)
			{
				item.get_label(data);
			};

		struct node_if
		{
			using children_type = std::vector<node_if>;
			using entry_cb_t = std::function<response_t(DataInterface&)>;
			using exit_cb_t = std::function<void()>;
			using event_cb_t = std::function<void(event_t)>;
			using conrifm_cb_t = std::function<response_t(DataInterface&, size_t)>;
			using render_cb_t = std::function<void()>;
			using get_label_t = std::function<void(DataInterface&)>;

			entry_cb_t on_entry;
			exit_cb_t on_exit;
			event_cb_t on_event;
			conrifm_cb_t on_confirm;
			render_cb_t on_render;
			get_label_t get_label;

			//node_if(entry_cb_t &&on_entry_, exit_cb_t &&on_exit_, event_cb_t &&on_event_, render_cb_t &&on_render_, get_label_t &&get_label_)
			//	: on_entry(on_entry_), on_exit(on_exit_), on_event(on_event_), on_render(on_render_), get_label(get_label_)
			//{}
			//node_if(node_if const& node)
			//	: on_entry(node.on_entry), on_exit(node.on_exit), on_event(node.on_event), on_render(node.on_render), get_label(node.get_label)
			//{}
			//node_if(node_if &&node)
			//	: on_entry(std::move(node.on_entry)), on_exit(std::move(node.on_exit)), on_event(std::move(node.on_event)), on_render(std::move(node.on_render)), get_label(std::move(node.get_label))
			//{}

		};

		struct DataInterface {

			char buffer[BufferRowMax][BufferColMax];
			size_t pos;

			impl::node_if* next_node;
			void* next_dyn_data;
			impl::node_if::children_type* children_ptr;
			size_t menu_count;

			DataInterface() : buffer{ 0 }, pos{ 0 }, next_node(nullptr), next_dyn_data(nullptr), children_ptr(nullptr), menu_count(0) {

			}
		};


		//template <typename Node>
		//auto make_node_if(Node& node)
		//{
		//	return node_if{
		//		[&](void* data) { return node_if_hdl_entry(node, data); },
		//		[&]() { node_if_hdl_exit(node); },
		//		[&](event_t event) { node_if_hdl_event(node, event); },
		//		[&]() { node_if_hdl_render(node); },
		//		[&](RenderBuffer& buffer) { node_if_get_label(node, buffer); }
		//	};
		//}


		template <typename Node>
		auto make_node_if(Node& node)
		{
			return node_if{
				[&](DataInterface& data) { return node_if_hdl_entry(node, data); },
				[&]() { node_if_hdl_exit(node); },
				[&](event_t event) { node_if_hdl_event(node, event); },
				[&](DataInterface& data, size_t index) { return node_if_hdl_confirm(node, data, index); },
				[&]() { node_if_hdl_render(node); },
				[&](DataInterface& data) { node_if_get_label(node, data); }
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


		template<typename Tuple, std::size_t... I>
		void init_node(Tuple& tuple, node_if::children_type& children, std::index_sequence<I...>) {
			// 子要素をすべてinit
			auto dummy = {
				(std::get<I>(tuple).init(), 0)...
			};
			// init後にchildを作成
			auto temp = tuple_to_vec(tuple);
			children.swap(temp);
		}
		template<typename Tuple>
		void init_node(Tuple& tuple, node_if::children_type& children) {
			init_node(tuple, children, std::make_index_sequence<std::tuple_size<Tuple>::value>());
		}


		template <typename... Args, std::size_t... I>
		constexpr size_t get_max_depth(std::tuple<Args...>& tuple, std::index_sequence<I...>)
		{
			return std::max({ std::get<I>(tuple).depth... });
		}
		template <typename... Args>
		constexpr size_t get_max_depth(std::tuple<Args...>& tuple) {
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

	template <typename Node>
	class menu_mng
	{
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
		impl::DataInterface data_;

	public:
		size_t depth;

	public:
		menu_mng(Node& node) = delete;

		menu_mng(Node&& node) : raw_tuple_{ std::forward<Node>(node) }, select_idx_(0), confirm_idx_(-1)
		{
			depth = node.depth;
			confirm_stack_.reserve(depth);
		}

		void init() {
			root_node_ = &(std::get<0>(raw_tuple_));
			// tuple内全ノードを初期化
			root_node_->init();
			// rootノードの操作I/Fを作成
			root_node_if_ = impl::make_node_if(*root_node_);
		}

		void on_entry()
		{
			// 初期化
			select_idx_ = 0;
			data_.pos = 0;
			confirm_stack_.clear();
			// rootノードを初期状態として登録
			data_.next_node = &root_node_if_;
			data_.next_dyn_data = nullptr;
			// rootノードに遷移処理
			entry_menu();
			// メニュー表示
			on_render();
		}

		void on_exit()
		{
			select_idx_ = 0;
			confirm_idx_ = -1;
		}
		void on_event(event_t event)
		{
			switch (event)
			{
			case event_t::None:
				break;
			case event_t::SelectNext:
				select_idx_++;
				if (select_idx_ >= children_count_)
				{
					select_idx_ = 0;
				}
				on_render();

				break;
			case event_t::SelectPrev:
				if (select_idx_ == 0) {
					select_idx_ = (int)children_count_ - 1;
				}
				else {
					select_idx_--;
				}
				on_render();
				break;
			case event_t::Confirm:
				// 選択中メニューで決定
				//check_confirm(buff, &(*children_ptr_)[select_idx_]);
				break;
			case event_t::Back:
				break;
			default:
				break;
			}
		}

		void on_confirm() {
			// 現ノードでconfirmイベントを実行
			auto resp = confirm_ptr_->on_confirm(data_, select_idx_);
			switch (resp) {
			case response_t::OK:
				// 操作受理でメニュー遷移
				entry_menu();
				break;

			case response_t::ReqBack:
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
			for (idx = 0; idx < data_.pos; idx++)
			{
				if (idx == select_idx_)
				{
					std::cout << "> ";
				}
				else
				{
					std::cout << "  ";
				}

				std::cout << data_.buffer[idx] << std::endl;
			}
			std::cout << std::endl;
		}

	private:
		void entry_menu() {
			// 遷移先ノード設定
			confirm_stack_.push_back(data_.next_node);
			confirm_ptr_ = data_.next_node;
			// 遷移処理
			// 子要素を持つなら処理する
			auto result = confirm_ptr_->on_entry(data_);
			children_ptr_ = data_.children_ptr;
			if (children_ptr_ != nullptr) {
				children_count_ = data_.menu_count;
			}
			else {
				children_count_ = 0;
			}
			// 表示バッファ更新
			make_disp();
		}

		void make_disp() {
			//
			if (children_ptr_ != nullptr) {
				// 初期化
				select_idx_ = 0;
				data_.pos = 0;
				// メニュー表示情報作成
				for (auto& node : *children_ptr_)
				{
					node.get_label(data_);
				}
			}
		}
	};

	template <typename H, typename Tuple>
	struct menu_node
	{
		using header_type = menu_header<H>;
		using tag = impl::tag_menu_node;
		using children_type = impl::node_if::children_type;
		static constexpr size_t ElemSize = 1;

		char const* label_;
		size_t label_len_;
		header_type header_;
		Tuple raw_tuple_;
		children_type children_;
		size_t children_count_;
		size_t depth;
		int tgt_;
		std::vector<size_t> index_table_;
		size_t confirm_index_;

		menu_node(char const* label, header_type&& header, Tuple&& tuple) : label_(label), header_(std::forward<header_type>(header)), raw_tuple_(std::forward<Tuple>(tuple)), children_count_(0), tgt_(-1)
		{
			label_len_ = std::strlen(label_);
			depth = impl::get_max_depth(raw_tuple_) + 1;
			auto array = impl::gen_array(raw_tuple_);
			index_table_.assign(std::begin(array), std::end(array));
		}

		//menu_node(menu_node& node) = delete;
		//menu_node(menu_node&& node) : label_(node.label_), header_(std::move(node.header_)), raw_tuple_(std::move(node.raw_tuple_)), tgt_(node.tgt_), label_len_(node.label_len_), depth(node.depth), children_(std::move(node.children_)) {
		//}

		void init() {
			impl::init_node(raw_tuple_, children_);
			children_count_ = children_.size();
		}

		response_t on_entry(impl::DataInterface& data)
		{
			data.children_ptr = &children_;
			return response_t::None;
		}
		void on_exit()
		{
		}
		void on_event(event_t event)
		{
		}
		response_t on_confirm(impl::DataInterface& data, size_t index) {
			// 範囲チェックはnode_mngで実施済み
			// childrenが存在するかだけチェックする
			if (children_count_ == 0) {
				return response_t::NG;
			}

			// 受理処理を実施
			confirm_index_ = index;
			size_t next_idx = index_table_[index];
			data.next_node = &children_[next_idx];
			data.next_dyn_data = nullptr;

			return response_t::OK;
		}
		void on_render()
		{
			//
			header_.on_render(nullptr);
		}

		void get_label(impl::DataInterface& data)
		{
			std::memcpy(data.buffer[data.pos], label_, label_len_ + 1);
			data.pos++;
		}
	};

	template <typename T, size_t N, typename H, typename Tuple>
	struct menu_node_dyn
	{
		using tag = impl::tag_menu_node;
		using data_type = T;
		using header_type = menu_header<H>;
		using children_type = impl::node_if::children_type;
		static constexpr size_t ElemSize = N;

		char const* label_;
		T(&cont_)[N];
		header_type header_;
		Tuple raw_tuple_;
		std::vector<impl::node_if> children_;
		size_t children_count_;
		size_t depth;
		int tgt_;
		std::vector<size_t> index_table_;

		//menu_node_dyn(menu_node_dyn& node) = delete;
		//menu_node_dyn(menu_node_dyn&& node) : label_(node.label_), cont_(node.cont_), raw_tuple_(std::move(node.raw_tuple_)), children_(std::move(node.children_)), depth(node.depth), tgt_(node.tgt_) {
		//}

		menu_node_dyn(T(&cont)[N], header_type&& header, Tuple&& tuple) : label_(nullptr), cont_(cont), header_(std::forward<header_type>(header)), raw_tuple_(std::forward<Tuple>(tuple)), children_count_(0), tgt_(-1)
		{
			depth = impl::get_max_depth(raw_tuple_) + 1;
			auto array = impl::gen_array(raw_tuple_);
			index_table_.assign(std::begin(array), std::end(array));
		}

		void init() {
			impl::init_node(raw_tuple_, children_);
			children_count_ = children_.size();
		}

		response_t on_entry(impl::DataInterface& data)
		{
			data.children_ptr = &children_;
			return response_t::None;
		}
		void on_exit()
		{
		}
		void on_event(event_t event)
		{
		}
		response_t on_confirm(impl::DataInterface& data, size_t index) {
			// 範囲チェック
			if (index >= N) {
				return response_t::NG;
			}
			// childrenが存在するかチェックする
			if (children_count_ == 0) {
				return response_t::NG;
			}
			// 選択データをチェック
			if (!(bool)(cont_[index])) {
				return response_t::NG;
			}

			return response_t::OK;
		}
		void on_render()
		{
			//
			header_.on_render(nullptr);
		}

		void get_label(impl::DataInterface& data)
		{
		}
	};

	template <typename Act>
	class menu_leaf
	{
	public:
		using tag = impl::tag_menu_leaf;
		using action_type = Act;
		using children_type = impl::node_if::children_type;
		static constexpr size_t depth = 1;
		static constexpr size_t ElemSize = 1;

	private:
		char const* label_;
		size_t label_len_;
		action_type actor_;

	public:
		menu_leaf(char const* label, Act& actor) = delete;
		menu_leaf(char const* label, Act&& actor) : label_(label), actor_(std::move(actor))
		{
			label_len_ = std::strlen(label_);
		}

		void init() {

		}

		char const* get_label() {
			return label_;
		}
		action_type& get_actor()
		{
			return actor_;
		}
		action_type&& move_actor() {
			return std::move(actor_);
		}

		response_t on_entry(impl::DataInterface& data)
		{
			data.children_ptr = nullptr;
			return response_t::None;
		}
		void on_exit()
		{
		}
		void on_event(event_t event)
		{
		}
		response_t on_confirm(impl::DataInterface& data, size_t index) {
			auto result = response_t::None;


			return result;
		}
		void on_render()
		{
		}

		void get_label(impl::DataInterface& data)
		{
			std::memcpy(data.buffer[data.pos], label_, label_len_ + 1);
			data.pos++;
		}
	};

	template <typename T, typename Act>
	class menu_leaf_dyn
	{
	public:
		using tag = impl::tag_menu_leaf;
		using action_type = Act;
		using data_type = T;
		using children_type = impl::node_if::children_type;
		static constexpr size_t depth = 1;
		static constexpr size_t ElemSize = 1;

	private:
		char const* label_;
		size_t label_len_;
		action_type actor_;
		data_type* data_;

	public:
		menu_leaf_dyn(menu_leaf<Act>& leaf) = delete;
		menu_leaf_dyn(menu_leaf<Act>&& leaf) : label_(leaf.get_label()), actor_(leaf.move_actor()), data_(nullptr)
		{
			label_len_ = std::strlen(label_);
		}

		void init() {

		}

		response_t on_entry(impl::DataInterface& data)
		{
			data_ = (data_type*)data.next_dyn_data;
			data.children_ptr = nullptr;
			return response_t::None;
		}
		void on_exit()
		{
		}
		void on_event(event_t event)
		{
		}
		response_t on_confirm(impl::DataInterface& data, size_t index) {
			auto result = response_t::None;


			return result;
		}
		void on_render()
		{
		}

		void get_label(impl::DataInterface& data)
		{
			std::memcpy(data.buffer[data.pos], label_, label_len_ + 1);
			data.pos++;
		}
	};

	namespace impl
	{
		template<typename T, typename Tuple, std::size_t I>
		auto make_menu_leaf_dyn(Tuple&& tuple) {
			using Elem = typename std::tuple_element<I, Tuple>::type;
			using Act = typename Elem::action_type;
			if constexpr (std::is_same_v<Elem, menu_leaf<Act>>) {
				return menu_leaf_dyn<T, Act>(std::forward<Elem>(std::get<I>(tuple)));
			}
			else {
				// error
			}
		}

		template <class T, class... Args, std::size_t... I>
		auto make_menu_leaf_dyn(T const& c, std::tuple<Args...>&& tuple, std::index_sequence<I...>)
		{
			return std::make_tuple(
				make_menu_leaf_dyn<T, std::tuple<Args...>, I>(std::forward<std::tuple<Args...>>(tuple))...
			);
			// return std::make_tuple(
			// 	menu_leaf_dyn<T, typename std::tuple_element<I, std::tuple<Args...>>::type::action_type>(
			// 		std::get<I>(tuple)
			// 	)...
			// );
		}
		template <class T, class... Args>
		auto make_menu_leaf_dyn(T const& c, std::tuple<Args...>&& tuple)
		{
			return make_menu_leaf_dyn(c, std::forward<std::tuple<Args...>>(tuple), std::make_index_sequence<std::tuple_size<std::tuple<Args...>>::value>());
		}
	}

	template <typename T, typename... Args>
	auto make_menu(menu_header<T>&& header, Args &&...nodes)
	{
		auto new_tuple = std::make_tuple(std::forward<Args>(nodes)...);
		auto node_ = menu_node<T, decltype(new_tuple)>{ "root", std::forward<menu_header<T>>(header), std::move(new_tuple) };
		//auto node_ = node("root", std::forward<menu_header<T>>(header), std::make_tuple(std::forward<Args>(nodes)...));
		return menu_mng<decltype(node_)>{std::move(node_)};
	}

	template <typename... Args>
	auto make_menu(Args &&...nodes)
	{
		return make_menu(menu_header<void>(), std::forward<Args>(nodes)...);
	}

	template <typename T, size_t N, typename U, typename... Args>
	auto node(T(&c)[N], menu_header<U>&& header, Args &&...nodes)
	{
		if constexpr (std::is_integral_v<T>)
		{
			// 固定文字列で表示するメニュー
			auto new_tuple = std::make_tuple(std::forward<Args>(nodes)...);
			return menu_node<U, decltype(new_tuple)>{c, std::forward<menu_header<U>>(header), std::move(new_tuple)};
		}
		else
		{
			// コンテナ(T(&c)[N])で動的に表示するメニュー
			auto new_tuple = impl::make_menu_leaf_dyn(c[0], std::make_tuple(std::forward<Args>(nodes)...));
			using result_type = menu_node_dyn<T, N, U, decltype(new_tuple)>;
			return result_type{ c, std::forward<menu_header<U>>(header), std::move(new_tuple) };
		}
	}

	template <typename T, size_t N, typename... Args>
	auto node(T(&c)[N], Args &&...nodes)
	{
		return node(c, menu_header<void>(), std::forward<Args>(nodes)...);
	}

	template <typename T>
	menu_leaf<T> leaf(char const* label, T&& actor)
	{
		return menu_leaf<T>{label, std::forward<T>(actor)};
	}

	template <typename T>
	menu_header<T> header(T&& actor)
	{
		return menu_header<T>{std::forward<T>(actor)};
	}
}


struct data_node {
	char const* label;

	operator bool() {
		return label != nullptr;
	}
};

data_node data_c[] = {
	data_node{"item1"},
	data_node{"item2"},
	data_node{"item3"},
};

struct header_root_menu
{
	void operator()(void* data) {
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

namespace lm = lib_menu;

auto menu_back = lm::leaf("[戻る]", 3);

auto menu_mng = lm::make_menu(
	lm::header(header_root_menu()),

	lm::node("menu_1",
		lm::header(header_menu_1()),

		lm::node(data_c,
			lm::leaf("dyn_menu_1", 1),
			lm::leaf("dyn_menu_2", 2),
			lm::leaf("dyn_menu_3", 3)
		),
		lm::leaf("menu_1_2", 3)
	),

	lm::node("menu_2",
		lm::leaf("menu_2_1", 4),
		lm::leaf("menu_2_2", 5),
		lm::leaf("menu_2_3", 6)
	),

	lm::node("menu_3",
		lm::node("menu_3_1",
			lm::leaf("menu_3_1_1", 7),
			lm::node("menu_3_1_2",
				lm::leaf("menu_3_1_2_1", 8),
				lm::leaf("menu_3_1_2_2", 9),
				lm::leaf("menu_3_1_2_3", 10)
			),
			lm::leaf("menu_3_1_3", 11)
		),
		lm::leaf("menu_3_2", 12)
	),

	lm::leaf("test", 13),
	lm::leaf("Version", 14)
);

void event_next()
{
	std::cout << "[event:[SelectNext]]" << std::endl;
	menu_mng.on_event(lm::event_t::SelectNext);
}
void event_prev()
{
	std::cout << "[event:[SelectPrev]]" << std::endl;
	menu_mng.on_event(lm::event_t::SelectPrev);
}
void event_confirm()
{
	std::cout << "[event:[Confirm]]" << std::endl;
	menu_mng.on_confirm();
}

int main() {
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

	return 0;
}
