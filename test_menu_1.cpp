
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


		auto node_if_hdl_entry = [](auto item, auto data)
		{
			item.on_entry(data);
		};
		auto node_if_hdl_exit = [](auto item)
		{
			item.on_exit();
		};
		auto node_if_hdl_event = [](auto item, event_t event)
		{
			item.on_event(event);
		};
		auto node_if_hdl_render = [](auto item)
		{
			item.on_render();
		};
		auto node_if_get_label = [](auto item, auto &buffer)
		{
			item.get_label(buffer);
		};
		struct node_if
		{
			using entry_cb_t = std::function<void(void*)>;
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

		template <typename... Args, std::size_t... I>
		auto tuple_to_vec(std::tuple<Args...> &tuple, std::index_sequence<I...>)
		{
			return std::vector< node_if >{
				node_if{
					[&](void *data) { node_if_hdl_entry(std::get<I>(tuple), data); },
					[&]() { node_if_hdl_exit(std::get<I>(tuple)); },
					[&](event_t event) { node_if_hdl_event(std::get<I>(tuple), event); },
					[&]() { node_if_hdl_render(std::get<I>(tuple)); },
					[&](RenderBuffer &buffer) { node_if_get_label(std::get<I>(tuple), buffer); }
				}...
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

	template <typename Tuple>
	class menu_mng
	{
		Tuple raw_tuple_;
		constexpr static size_t children_size_ = std::tuple_size<Tuple>::value;
		std::vector<impl::node_if> children_;
		int select_idx_;
		int confirm_idx_;
		std::vector<impl::node_if*> confirm_stack_;

	public:
		size_t depth;

	public:
		menu_mng(Tuple &&tuple) : raw_tuple_(tuple), select_idx_(0), confirm_idx_(-1)
		{
			depth = impl::get_max_depth(raw_tuple_);
			children_ = impl::tuple_to_vec(raw_tuple_);
			confirm_stack_.reserve(depth);
		}

		template<typename T>
		void on_entry(T& buffer)
		{
			for (auto &node : children_) {
				node.get_label(buffer);
			}
		}

		void on_exit()
		{
			select_idx_ = 0;
			confirm_idx_ = -1;
		}
		void on_event(event_t event)
		{
		}
		void on_render()
		{
		}

		void exec(std::size_t idx) {
			size_t i;
			for (i = 0; i < children_size_; i++)
			{
				//std::cout << children_[i].get_label() << std::endl;
			}
			std::cout << std::endl;
		}
	};

	template <typename Tuple>
	struct menu_node
	{
		using tag = impl::tag_menu_node;
		char const *label_;
		size_t label_len_;
		Tuple raw_tuple_;
		std::vector<impl::node_if> children_;
		size_t depth;
		int tgt_;

		menu_node(char const* label, Tuple &&tuple) : label_(label), raw_tuple_(tuple), tgt_(-1)
		{
			label_len_ = std::strlen(label_);
			depth = impl::get_max_depth(raw_tuple_) + 1;
			children_ = impl::tuple_to_vec(raw_tuple_);
		}

		void on_entry(void* ptr)
		{
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

	template <typename T, size_t N, typename Tuple>
	struct menu_node_dyn
	{
		using tag = impl::tag_menu_node;
		using data_type = T;

		char const *label_;
		T(&cont_)[N];
		Tuple raw_tuple_;
		std::vector<impl::node_if> children_;
		size_t depth;
		int tgt_;

		menu_node_dyn(T(&cont)[N], Tuple &&tuple) : label_(nullptr), cont_(cont), raw_tuple_(tuple), tgt_(-1)
		{
			depth = impl::get_max_depth(raw_tuple_) + 1;
			children_ = impl::tuple_to_vec(raw_tuple_);
		}

		void on_entry(void *data)
		{
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
		}
	};

	template <typename Act>
	class menu_leaf
	{
	public:
		using tag = impl::tag_menu_leaf;
		using action_type = Act;
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

		void on_entry(void *data)
		{
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

		void on_entry(void *data)
		{
			data_ = (data_type*)data;
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

	};

	namespace impl {

		template <typename... Args>
		struct make_menu_result_t {
			using type = menu_mng<std::tuple<Args...>>;
		};

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
		using result_type = typename impl::make_menu_result_t<Args...>::type;
		return result_type{std::make_tuple(std::forward<Args>(nodes)...)};
	}

	template <typename... Args>
	auto make_menu(Args &&...nodes)
	{
		using result_type = typename impl::make_menu_result_t<Args...>::type;
		return result_type{std::make_tuple(std::forward<Args>(nodes)...)};
	}

	template <typename T, size_t N, typename U, typename... Args>
	auto node(T(&c)[N], menu_header<U> &&header, Args &&...nodes)
	{
		if constexpr (std::is_integral_v<T>)
		{
			// 固定文字列で表示するメニュー
			auto new_tuple = std::make_tuple(std::forward<Args>(nodes)...);
			return menu_node<decltype(new_tuple)>{c, std::move(new_tuple)};
		}
		else
		{
			// コンテナ(T(&c)[N])で動的に表示するメニュー
			auto new_tuple = impl::make_menu_leaf_dyn(c[0], std::make_tuple(std::forward<Args>(nodes)...));
			using result_type = menu_node_dyn<T, N, decltype(new_tuple)>;
			return result_type{c, std::move(new_tuple)};
		}
	}

	template <typename T, size_t N, typename... Args>
	auto node(T(&c)[N], Args &&...nodes)
	{
		if constexpr (std::is_integral_v<T>)
		{
			// 固定文字列で表示するメニュー
			auto new_tuple = std::make_tuple(std::forward<Args>(nodes)...);
			return menu_node<decltype(new_tuple)>{c, std::move(new_tuple)};
		}
		else
		{
			// コンテナ(T(&c)[N])で動的に表示するメニュー
			auto new_tuple = impl::make_menu_leaf_dyn(c[0], std::make_tuple(std::forward<Args>(nodes)...));
			using result_type = menu_node_dyn<T, N, decltype(new_tuple)>;
			return result_type{c, std::move(new_tuple)};
		}
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

namespace lm = lib_menu;

auto menu_mng = lm::make_menu(
	lm::header(int(0)),

	lm::node("menu_1",
		lm::header(int(111)),

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

int main() {
	lm::RenderBuffer buff;

	menu_mng.on_entry(buff);

	{
		size_t idx;
		for (idx = 0; idx < buff.pos; idx++) {
			std::cout << buff.buffer[idx] << std::endl;
		}
	}

	return 0;
}
