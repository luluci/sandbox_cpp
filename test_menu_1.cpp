
#include <vector>
#include <tuple>
#include <functional>
#include <iostream>
#include <type_traits>
#include <algorithm>
#include <cstring>

namespace lib_menu {

	struct RenderBuffer {
		static constexpr size_t RowMax = 50;
		static constexpr size_t ColMax = 255;

		char buffer[RowMax][ColMax];
		size_t pos;

		RenderBuffer() : pos{ 0 } {

		}
	};

	template <typename Tuple>
	void add_buffer(Tuple &tuple)
	{
	}
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

	namespace impl
	{
		struct tag_menu_node {};
		struct tag_menu_leaf {};
		struct tag_menu_header {};


		auto node_if_hdl_entry = [](auto &item, auto data)
		{
			return item.on_entry(data);
		};
		auto node_if_hdl_exit = [](auto &item)
		{
			item.on_exit();
		};
		auto node_if_hdl_event = [](auto &item, event_t event)
		{
			item.on_event(event);
		};
		auto node_if_hdl_render = [](auto &item)
		{
			item.on_render();
		};
		auto node_if_get_label = [](auto &item, auto &buffer)
		{
			item.get_label(buffer);
		};
		struct node_if
		{
			using children_type = std::vector<node_if>;
			using entry_cb_t = std::function<children_type*(void *)>;
			using exit_cb_t = std::function<void()>;
			using event_cb_t = std::function<void(event_t)>;
			using render_cb_t = std::function<void()>;
			using get_label_t = std::function<void(RenderBuffer &)>;

			entry_cb_t on_entry;
			exit_cb_t on_exit;
			event_cb_t on_event;
			render_cb_t on_render;
			get_label_t get_label;
		};

		template <typename Node>
		auto make_node_if(Node &&node)
		{
			return node_if{
				[&](void *data) { return node_if_hdl_entry(std::forward<Node>(node), data); },
				[&]() { node_if_hdl_exit(std::forward<Node>(node)); },
				[&](event_t event) { node_if_hdl_event(std::forward<Node>(node), event); },
				[&]() { node_if_hdl_render(std::forward<Node>(node)); },
				[&](RenderBuffer &buffer) { node_if_get_label(std::forward<Node>(node), buffer); }
			};
		}

		template <typename... Args, std::size_t... I>
		auto tuple_to_vec(std::tuple<Args...> &tuple, std::index_sequence<I...>)
		{
			return std::vector< node_if >{
				make_node_if(std::get<I>(tuple))...
			};
		}
		template <typename... Args>
		auto tuple_to_vec(std::tuple<Args...> &tuple)
		{
			return tuple_to_vec(tuple, std::make_index_sequence<std::tuple_size<std::tuple<Args...>>::value>());
		}

		template <typename... Args, std::size_t... I>
		constexpr size_t get_max_depth(std::tuple<Args...> &tuple, std::index_sequence<I...>)
		{
			return std::max({ std::get<I>(tuple).depth... });
		}
		template <typename... Args>
		constexpr size_t get_max_depth(std::tuple<Args...> &tuple) {
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
		menu_header(Act &&actor) : actor_(std::move(actor))
		{
		}

		void on_render(void *data)
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
		menu_header(menu_header<void> &&)
		{
		}

		void on_render(void*)
		{
		}
	};

	template <typename Node>
	class menu_mng
	{
		Node node_;
		impl::node_if self_;
		int select_idx_;
		int confirm_idx_;
		std::vector<impl::node_if*> confirm_stack_;
		impl::node_if* confirm_ptr_;
		impl::node_if::children_type* children_ptr_;
		size_t children_count_;

	public:
		size_t depth;

	public:
		menu_mng(Node &&node) : node_(std::forward<Node>(node)), select_idx_(0), confirm_idx_(-1)
		{
			depth = node_.depth;
			confirm_stack_.reserve(depth);
			self_ = impl::make_node_if(node_);
			//
			confirm_stack_.push_back(&self_);
			confirm_ptr_ = &self_;
			children_ptr_ = confirm_ptr_->on_entry(nullptr);
			children_count_ = children_ptr_->size();
		}

		void on_entry(RenderBuffer &buffer)
		{
			//
			if (children_ptr_ != nullptr) {
				// メニュー表示情報作成
				for (auto &node : *children_ptr_)
				{
					node.get_label(buffer);
				}
				// 
				confirm_ptr_->on_render();
				// メニュー表示
				on_render(buffer);
			}
		}

		void on_exit()
		{
			select_idx_ = 0;
			confirm_idx_ = -1;
		}
		void on_event(event_t event, RenderBuffer &buff)
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
				on_render(buff);

				break;
			case event_t::SelectPrev:
				if (select_idx_ == 0) {
					select_idx_ = children_count_ - 1;
				}else{
					select_idx_--;
				}
				on_render(buff);
				break;
			case event_t::Confirm:
				// 選択中メニューで決定
				check_confirm(buff, &(*children_ptr_)[select_idx_]);
				break;
			case event_t::Back:
				break;
			default:
				break;
			}
		}

		void on_render(RenderBuffer &buff)
		{
			std::cout << "[menu]" << std::endl;
			size_t idx;
			for (idx = 0; idx < buff.pos; idx++)
			{
				if (idx == select_idx_)
				{
					std::cout << "> ";
				}else
				{
					std::cout << "  ";
				}

				std::cout << buff.buffer[idx] << std::endl;
			}
			std::cout << std::endl;
		}

	private:
		void check_confirm(RenderBuffer &buff, impl::node_if *child)
		{
			auto result = child->on_entry(nullptr);
			if (result != nullptr)
			{
				// メニューに入る
				confirm_stack_.push_back(child);
				confirm_ptr_ = child;
				children_ptr_ = result;
				children_count_ = children_ptr_->size();
				//
				on_render(buff);
			}
		}
	};

	template <typename H, typename Tuple>
	struct menu_node
	{
		using header_type = menu_header<H>;
		using tag = impl::tag_menu_node;
		using children_type = impl::node_if::children_type;
		char const *label_;
		size_t label_len_;
		header_type header_;
		Tuple raw_tuple_;
		children_type children_;
		size_t depth;
		int tgt_;

		menu_node(char const *label, header_type &&header, Tuple &&tuple) : label_(label), header_(std::forward<header_type>(header)), raw_tuple_(std::forward<Tuple>(tuple)), tgt_(-1)
		{
			label_len_ = std::strlen(label_);
			depth = impl::get_max_depth(raw_tuple_) + 1;
			children_ = impl::tuple_to_vec(raw_tuple_);
		}

		children_type* on_entry(void *ptr)
		{
			return &children_;
		}
		void on_exit()
		{
		}
		void on_event(event_t event)
		{
		}
		void on_render()
		{
			//
			header_.on_render(nullptr);
		}

		void get_label(RenderBuffer &buffer)
		{
			std::memcpy(buffer.buffer[buffer.pos], label_, label_len_ + 1);
			buffer.pos++;
		}
	};

	template <typename T, size_t N, typename H, typename Tuple>
	struct menu_node_dyn
	{
		using tag = impl::tag_menu_node;
		using data_type = T;
		using header_type = menu_header<H>;
		using children_type = impl::node_if::children_type;

		char const *label_;
		T(&cont_)[N];
		header_type header_;
		Tuple raw_tuple_;
		std::vector<impl::node_if> children_;
		size_t depth;
		int tgt_;

		menu_node_dyn(T (&cont)[N], header_type &&header, Tuple &&tuple) : label_(nullptr), cont_(cont), header_(std::forward<header_type>(header)), raw_tuple_(std::forward<Tuple>(tuple)), tgt_(-1)
		{
			depth = impl::get_max_depth(raw_tuple_) + 1;
			children_ = impl::tuple_to_vec(raw_tuple_);
		}

		children_type* on_entry(void *data)
		{
			return &children_;
		}
		void on_exit()
		{
		}
		void on_event(event_t event)
		{
		}
		void on_render()
		{
			//
			header_.on_render(nullptr);
		}

		void get_label(RenderBuffer &buffer)
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

	private:
		char const *label_;
		size_t label_len_;
		action_type actor_;

	public:
		menu_leaf(char const *label, Act &actor) : label_(label), actor_(actor)
		{
			label_len_ = std::strlen(label_);
		}
		menu_leaf(char const *label, Act &&actor) : label_(label), actor_(std::move(actor))
		{
			label_len_ = std::strlen(label_);
		}

		char const* get_label() {
			return label_;
		}
		action_type &get_actor()
		{
			return actor_;
		}
		action_type&& move_actor() {
			return std::move(actor_);
		}

		children_type* on_entry(void *data)
		{
			return nullptr;
		}
		void on_exit()
		{
		}
		void on_event(event_t event)
		{
		}
		void on_render()
		{
		}

		void get_label(RenderBuffer &buffer)
		{
			std::memcpy(buffer.buffer[buffer.pos], label_, label_len_ + 1);
			buffer.pos++;
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

	private:
		char const *label_;
		size_t label_len_;
		action_type actor_;
		data_type *data_;

	public:
		menu_leaf_dyn(menu_leaf<Act> &leaf) : label_(leaf.get_label()), actor_(leaf.get_actor()), data_(nullptr)
		{
			label_len_ = std::strlen(label_);
		}
		menu_leaf_dyn(menu_leaf<Act> &&leaf) : label_(leaf.get_label()), actor_(leaf.move_actor()), data_(nullptr)
		{
			label_len_ = std::strlen(label_);
		}

		children_type* on_entry(void *data)
		{
			data_ = (data_type*)data;
			return nullptr;
		}
		void on_exit()
		{
		}
		void on_event(event_t event)
		{
		}
		void on_render()
		{
		}

		void get_label(RenderBuffer &buffer)
		{
			std::memcpy(buffer.buffer[buffer.pos], label_, label_len_ + 1);
			buffer.pos++;
		}
	};

	namespace impl
	{
		template<typename T, typename Tuple, std::size_t I>
		auto make_menu_leaf_dyn(Tuple &&tuple) {
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
		auto make_menu_leaf_dyn(T const &c, std::tuple<Args...> &&tuple, std::index_sequence<I...>)
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
		auto make_menu_leaf_dyn(T const &c, std::tuple<Args...> &&tuple)
		{
			return make_menu_leaf_dyn(c, std::forward<std::tuple<Args...>>(tuple), std::make_index_sequence<std::tuple_size<std::tuple<Args...>>::value>());
		}
	}

	template <typename T, typename... Args>
	auto make_menu(menu_header<T> &&header, Args &&...nodes)
	{
		auto new_tuple = std::make_tuple(std::forward<Args>(nodes)...);
		auto node_ = menu_node<T, decltype(new_tuple)>{"root", std::forward<menu_header<T>>(header), std::move(new_tuple)};
		//auto node_ = node("root", std::forward<menu_header<T>>(header), std::make_tuple(std::forward<Args>(nodes)...));
		return menu_mng<decltype(node_)>(std::move(node_));
	}

	template <typename... Args>
	auto make_menu(Args &&...nodes)
	{
		return make_menu(menu_header<void>(), std::forward<Args>(nodes)...);
	}

	template <typename T, size_t N, typename U, typename... Args>
	auto node(T(&c)[N], menu_header<U> &&header, Args &&...nodes)
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
			return result_type{c, std::forward<menu_header<U>>(header), std::move(new_tuple)};
		}
	}

	template <typename T, size_t N, typename... Args>
	auto node(T(&c)[N], Args &&...nodes)
	{
		return node(c, menu_header<void>(), std::forward<Args>(nodes)...);
	}

	template <typename T>
	menu_leaf<T> leaf(char const *label, T &&actor)
	{
		return menu_leaf<T>{label, actor};
	}

	template <typename T>
	menu_header<T> header(T &&actor)
	{
		return menu_header<T>{std::forward<T>(actor)};
	}
}


struct data_node {
	char const* label;

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
	void operator()(void *data)
	{
		std::cout << "<header_menu_1>" << std::endl;
	}
};

namespace lm = lib_menu;

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

lm::RenderBuffer buff;

void event_next()
{
	std::cout << "[event:[SelectNext]]" << std::endl;
	menu_mng.on_event(lm::event_t::SelectNext, buff);
}
void event_prev()
{
	std::cout << "[event:[SelectPrev]]" << std::endl;
	menu_mng.on_event(lm::event_t::SelectPrev, buff);
}
void event_confirm()
{
	std::cout << "[event:[Confirm]]" << std::endl;
	menu_mng.on_event(lm::event_t::Confirm, buff);
}

int main() {

	menu_mng.on_entry(buff);

	event_next();
	event_next();
	event_next();
	event_next();
	event_next();
	event_prev();
	event_next();
	event_confirm();

	return 0;
}
