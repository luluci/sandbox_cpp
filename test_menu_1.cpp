
#include <vector>
#include <tuple>
#include <functional>
#include <iostream>
#include <type_traits>

namespace lib_menu {

	enum class event_t {
		None,
		Select,
		Back,

		MAX,
	};

	namespace impl
    {
        auto node_if_hdl_entry = [](auto item)
        {
            // std::cout << item.label << std::endl;
        };
        auto node_if_hdl_exit = [](auto item)
        {
            // std::cout << item.label << std::endl;
        };
        auto node_if_hdl_event = [](auto item, event_t event)
        {
            // std::cout << item.label << std::endl;
        };
        auto node_if_hdl_render = [](auto item)
        {
            // std::cout << item.label << std::endl;
        };
        auto node_if_get_label = [](auto item)
        {
            return item.get_label();
        };
        struct node_if
        {
            using entry_cb_t = std::function<void()>;
            using exit_cb_t = std::function<void()>;
            using event_cb_t = std::function<void(event_t)>;
            using render_cb_t = std::function<void()>;
            using get_label_t = std::function<char const *()>;

            entry_cb_t on_entry;
            exit_cb_t on_exit;
            event_cb_t on_event;
            render_cb_t on_render;
            get_label_t get_label;
        };

        template <class... Args, std::size_t... I>
        auto tuple_to_vec(std::tuple<Args...> &tuple, std::index_sequence<I...>)
        {
            return std::vector< node_if >{
                node_if{
                    [&](){ node_if_hdl_entry(std::get<I>(tuple)); },
                    [&](){ node_if_hdl_exit(std::get<I>(tuple)); },
                    [&](event_t event){ node_if_hdl_event(std::get<I>(tuple), event); },
                    [&](){ node_if_hdl_render(std::get<I>(tuple)); },
                    [&](){ return node_if_get_label(std::get<I>(tuple)); }
                }...
            };
			// return std::vector<std::function<void(event_t)>>{
			// 	([&](event_t event)
			// 	{ function(std::get<I>(tuple), event); })...};
			//return std::vector<std::function<void()>>(functions);
		}
		template <class... Args>
		auto tuple_to_vec(std::tuple<Args...> &tuple)
		{
			return tuple_to_vec(tuple, std::make_index_sequence<std::tuple_size<std::tuple<Args...>>::value>());
		}

        auto node_if_dyn_hdl_entry = [](auto item, auto data)
        {
            item.on_entry(data);
        };
        auto node_if_dyn_hdl_exit = [](auto item)
        {
            item.on_exit();
        };
        auto node_if_dyn_hdl_event = [](auto item, event_t event)
        {
            item.on_event(event);
        };
        auto node_if_dyn_hdl_render = [](auto item)
        {
            item.on_render();
        };
        auto node_if_dyn_get_label = [](auto item)
        {
            return item.get_label();
        };
        template<typename T>
        struct node_if_dyn
        {
            using entry_cb_t = std::function<void(T*)>;
            using exit_cb_t = std::function<void()>;
            using event_cb_t = std::function<void(event_t)>;
            using render_cb_t = std::function<void()>;
            using get_label_t = std::function<char const*()>;

            entry_cb_t on_entry;
            exit_cb_t on_exit;
            event_cb_t on_event;
            render_cb_t on_render;
            get_label_t get_label;
        };

        template <typename T, typename... Args, std::size_t... I>
        auto tuple_to_vec_dyn(std::tuple<Args...> &tuple, T &hndl, std::index_sequence<I...>)
        {
            return std::vector< node_if_dyn<T> >{
                node_if_dyn<T>{
                    [&](T *data){ node_if_dyn_hdl_entry(std::get<I>(tuple), data); },
                    [&](){ node_if_dyn_hdl_exit(std::get<I>(tuple)); },
                    [&](event_t event){ node_if_dyn_hdl_event(std::get<I>(tuple), event); },
                    [&](){ node_if_dyn_hdl_render(std::get<I>(tuple)); },
                    [&](){ return node_if_get_label(std::get<I>(tuple)); }
                }...
            };
            //([&](event_t event)
            // { function(std::get<I>(tuple), event); })...
            //return std::vector<std::function<void()>>(functions);
		}
        template <typename T, typename... Args>
        auto tuple_to_vec_dyn(std::tuple<Args...> &tuple, T &hndl)
        {
            return tuple_to_vec_dyn(tuple, hndl, std::make_index_sequence<std::tuple_size<std::tuple<Args...>>::value>());
        }

        template <typename Tuple>
		struct node
		{
			char const *label;
			Tuple raw_tuple_;
		};

		template <typename T, size_t N, typename Tuple>
		struct node_dyn
		{
			T(&cont_)[N];
			Tuple raw_tuple_;
		};

        template <typename... Args, std::size_t... I>
        constexpr size_t get_max_depth(std::tuple<Args...> &tuple, std::index_sequence<I...>)
        {
            return std::max({std::get<I>(tuple).depth...});
        }
        template <typename... Args>
        constexpr size_t get_max_depth(std::tuple<Args...> &tuple){
            return get_max_depth(tuple, std::make_index_sequence<std::tuple_size<std::tuple<Args...>>::value>());
        }
    }

    template <typename Tuple>
	class menu_mng
	{
		Tuple raw_tuple_;
		constexpr static size_t children_size_ = std::tuple_size<Tuple>::value;
        std::vector<impl::node_if> children_;
        int tgt_;

    public:
        size_t depth;

    public:
		menu_mng(Tuple &&tuple) : raw_tuple_(tuple), tgt_(-1)
        {
            depth = impl::get_max_depth(raw_tuple_);
            children_ = impl::tuple_to_vec(raw_tuple_);
        }

        void exec(std::size_t idx) {
            size_t i;
            for (i=0; i<children_size_; i++)
            {
                std::cout << children_[i].get_label() << std::endl;
            }
            std::cout << std::endl;
        }
	};

    template <typename Tuple>
    struct menu_node
    {
        char const *label_;
        Tuple raw_tuple_;
        std::vector<impl::node_if> children_;
        size_t depth;
        int tgt_;

        menu_node(char const* label, Tuple &&tuple) : label_(label), raw_tuple_(tuple), tgt_(-1)
        {
            depth = impl::get_max_depth(raw_tuple_) + 1;
            children_ = impl::tuple_to_vec(raw_tuple_);
        }

        void on_entry()
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

        char const *get_label()
        {
            return label_;
        }
    };

    template <typename T, size_t N, typename Tuple>
    struct menu_node_dyn
    {
        char const *label_;
        T(&cont_)[N];
        Tuple raw_tuple_;
        std::vector<impl::node_if_dyn<T>> children_;
        size_t depth;
        int tgt_;

        menu_node_dyn(T (&cont)[N], Tuple &&tuple) : label_(nullptr), cont_(cont), raw_tuple_(tuple), tgt_(-1)
        {
            depth = impl::get_max_depth(raw_tuple_) + 1;
            children_ = impl::tuple_to_vec_dyn(raw_tuple_, cont_[0]);
        }

        void on_entry()
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

        char const *get_label()
        {
            return label_;
        }
    };

    template <typename Act>
    class menu_leaf
    {
    public:
        using action_type = Act;
        static constexpr size_t depth = 1;

    private:
        char const *label_;
        action_type actor_;

    public:
        menu_leaf(char const *label, Act &actor) : label_(label), actor_(actor)
        {
        }
        menu_leaf(char const *label, Act &&actor) : label_(label), actor_(std::move(actor))
        {
        }

        action_type &get_actor()
        {
            return actor_;
        }
        action_type&& move_actor() {
            return std::move(actor_);
        }

        void on_entry()
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

        char const *get_label()
        {
            return label_;
        }
    };

    template <typename T, typename Act>
	class menu_leaf_dyn
    {
    public:
        using action_type = Act;
        using data_type = T;
        static constexpr size_t depth = 1;

    private:
        char const *label_;
        action_type actor_;
        data_type *data_;

    public:
        menu_leaf_dyn(menu_leaf<Act> &leaf) : label_(nullptr), actor_(leaf.get_actor()), data_(nullptr)
        {
		}
        menu_leaf_dyn(menu_leaf<Act> &&leaf) : label_(nullptr), actor_(leaf.move_actor()), data_(nullptr)
        {
		}

        void on_entry(data_type *data)
        {
            data_ = data;
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

        char const *get_label() {
            return label_;
        }
    };

    namespace impl {

		template <typename... Args>
		struct make_menu_result_t {
			using type = menu_mng<std::tuple<Args...>>;
		};

		template <typename... Args>
		struct make_node_result_t
		{
			using type = node<std::tuple<Args...>>;
		};

		template <typename T, size_t N, typename... Args>
		struct make_node_dyn_result_t
		{
			using type = node_dyn<T, N, std::tuple<Args...>>;
		};

        template<typename T, typename Tuple, std::size_t I>
        auto make_menu_leaf_dyn(Tuple &&tuple) {
            using Elem = typename std::tuple_element<I, Tuple>::type;
            using Act = typename Elem::action_type;
            if constexpr (std::is_same_v<Elem, menu_leaf<Act>>) {
                return menu_leaf_dyn<T, Act>(std::forward<Elem>(std::get<I>(tuple)));
            } else {
                // error
            }
        }

		template <class T, class... Args, std::size_t... I>
		auto make_menu_leaf_dyn(T const &c, std::tuple<Args...> &&tuple, std::index_sequence<I...>)
		{
			return std::make_tuple(
				make_menu_leaf_dyn<T,std::tuple<Args...>,I>(std::forward<std::tuple<Args...>>(tuple))...
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

	template <typename... Args>
	auto make_menu(Args &&...nodes) -> typename impl::make_menu_result_t<Args...>::type
	{
		using result_type = typename impl::make_menu_result_t<Args...>::type;
		return result_type{ std::make_tuple(std::forward<Args>(nodes)...) };
	}

	template <typename T, size_t N, typename... Args>
	auto node(T(&c)[N], Args &&...nodes)
	{
		if constexpr (std::is_integral_v<T>) {
			// 固定文字列で表示するメニュー
            auto new_tuple = std::make_tuple(std::forward<Args>(nodes)...);
            return menu_node<decltype(new_tuple)>{c, std::move(new_tuple)};
        }
		else {
			// コンテナ(T(&c)[N])で動的に表示するメニュー
			auto new_tuple = impl::make_menu_leaf_dyn(c[0], std::make_tuple(std::forward<Args>(nodes)...));
            using result_type = menu_node_dyn<T, N, decltype(new_tuple)>;
            return result_type{c, std::move(new_tuple)};
        }

	}

	template<typename T>
	menu_leaf<T> leaf(char const* label, T&& actor) {
        return menu_leaf<T>{label, actor};
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

	lm::node("menu_1",
		lm::node(data_c,
			lm::leaf("dyn_menu_1", 1),
			lm::leaf("dyn_menu_2", 2),
			lm::leaf("dyn_menu_3", 3)
		)
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

	menu_mng.exec(0);
	//menu_mng.exec(1);
	//menu_mng.exec(2);

	return 0;
}
