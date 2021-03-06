#ifndef xui_api_object_base
#define xui_api_object_base

namespace xui {
	// Wrapper for unique_ptr that requires tTy to be a class.
	template < typename tTy = xui::object_base > requires std::is_class < tTy >::value
	using unique_object_ptr = std::unique_ptr < tTy >;

	// Any child type vector.
	template < typename tTy >
	using child_vector = std::vector < unique_object_ptr < tTy > >;

	// Object flag markers.
	enum object_flags {
		// Is disabled by parent.
		OBJECT_FLAG_DISABLED ,
		// Is hidden from view.
		OBJECT_FLAG_HIDDEN ,
		// Is being hovered.
		OBJECT_FLAG_HOVERED ,
		// Is in interaction.
		OBJECT_FLAG_INTERACTION ,
		// Is able to use the command,
		// serves more as an 'is awake' flag.
		OBJECT_FLAG_COGITABLE
	};

	class object_base {
	protected:
		// Is this object focused.
		template < typename tTy > requires std::is_pointer < tTy >::value
			std::optional < bool > any_focus ( tTy& parent ) const {
			// Is there an actively focused object.
			if ( parent->focused ( ) )
				// Is this object focused.
				return parent->focused ( ) == this;

			// No object has focus.
			return std::nullopt;
		};

		// Sets parents focus on self.
		template < typename tTy > requires std::is_pointer < tTy >::value
		auto self_focus ( tTy& parent ) {
			// No object has focus.
			if ( auto focus = any_focus ( parent ); !focus.has_value ( ) )
				// Set self to focus.
				parent->focused ( ) = this;
		};

		// Sets parents focus on self.
		template < typename tTy > requires std::is_pointer < tTy >::value
		auto self_unfocus ( tTy& parent ) {
			// Ensure we're the object that's focused.
			if ( auto focus = any_focus ( parent ); focus.has_value ( ) && *focus )
				// Empty focus object.
				parent->focused ( ) = { };
		};
	public:
		// Constructor.
		object_base ( xui::vector_2d <> location , xui::vector_2d <> size ) 
					: m_Location { location } , m_Size { size } { };

		// Default constructor.
		object_base ( void ) : m_Location { } , m_Size { } { };

		// Deconstructor.
		~object_base ( void ) = default;

		// Location and size of object.
		xui::vector_2d <> m_Location , m_Size;

		// Object active flags.
		std::bitset < 5 > m_Flags;

		// Api.
		xui::details::base_api* m_Api_ptr;

		// Process input for object.
		virtual void input ( xui::input_command& command ) = 0;

		// Render object.
		virtual void render ( void ) = 0;
	};

	// Marks that an object is an immediate child of tTy.
	template < typename tTy > requires std::is_class < tTy >::value
	class immediate_child_of : public xui::object_base {
	public:
		// Parent.
		tTy* m_Parent_ptr;

		immediate_child_of ( xui::vector_2d <> size ) :
			xui::object_base ( { }  , size ) { };

		// Constructor/deconstructor.
		immediate_child_of ( void ) : xui::object_base ( ) { };
		~immediate_child_of ( void ) = default;
	};

	// Only allows children that are immediate to tTy.
	template < typename tTy = xui::object_base >
	using immediate_children_vector = child_vector < xui::immediate_child_of < tTy > >;

	template < typename tTy > requires std::is_class < tTy >::value
	class immediate_parent {
	protected:
		// Actively focused child object.
		xui::object_base* m_Focused_ptr;

		// Children objects unique ptrs.
		xui::immediate_children_vector < tTy > m_Children_ptrs;
	public:
		immediate_parent ( void ) : m_Focused_ptr { } { };
		~immediate_parent ( void ) = default;

		// Add child to tTy.
		template < typename stTy >
		requires std::is_base_of < xui::immediate_child_of < tTy > , stTy >::value
			auto add_child ( xui::unique_object_ptr < stTy > object ) {
			m_Children_ptrs.push_back ( std::move ( object ) );
		};

		// Get active ptr.
		auto& focused ( void ) {
			return m_Focused_ptr;
		};
	};
}; // !!! xui

#endif // !!! xui_api_object_base
