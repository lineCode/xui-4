#include <xui/xui.hpp>

// g_Api.
std::unique_ptr < xui::details::global_api > xui::g_Api;

// Wndproc implementation.
LRESULT _stdcall wndproc_impl ( HWND hwnd , UINT msg , WPARAM wparam , LPARAM lparam ) {
	// XUI input processing.
	if ( xui::g_Api->input_distribution ( )->process ( hwnd , msg , wparam , lparam ) )
		return TRUE;

	// Return original wndproc.
	return CallWindowProcA ( xui::g_Api->input_distribution ( )->wndproc ( ) , hwnd , msg , wparam , lparam );
};

// Construction.
xui::details::input_distribution::input_distribution ( HWND hwnd ) : m_Hwnd { hwnd } {
	// Get wndproc.
	m_Wndproc = reinterpret_cast < WNDPROC > ( SetWindowLongA ( m_Hwnd , GWLP_WNDPROC , reinterpret_cast < LONG > ( wndproc_impl ) ) );
};

// Deconstruction.
xui::details::input_distribution::~input_distribution ( void ) {
	// Reset wndproc.
	SetWindowLongA ( m_Hwnd , GWLP_WNDPROC , reinterpret_cast < LONG > ( m_Wndproc ) );
};

// distribution of input command.
auto xui::details::input_distribution::distribute ( xui::input_command& command ) {
	bool cogitation { false };

	// Has no children.
	if ( xui::g_Api->m_Children_ptrs.empty ( ) )
		return cogitation;

	for ( auto& next_child : xui::g_Api->m_Children_ptrs ) {
		// Set api ptr.
		if ( !next_child->m_Api_ptr )
			next_child->m_Api_ptr = xui::g_Api.get ( );

		// Disable other objects while one has taken focus.
		if ( xui::g_Api->m_Focused_ptr && xui::g_Api->m_Focused_ptr != next_child.get ( ) )
			next_child->m_Flags.set ( xui::OBJECT_FLAG_DISABLED , TRUE );

		// Re-enable objects that were formerly disabled.
		if ( !xui::g_Api->m_Focused_ptr && next_child->m_Flags.test ( xui::OBJECT_FLAG_DISABLED ) )
			next_child->m_Flags.flip ( xui::OBJECT_FLAG_DISABLED );

		// Run objects input against command.
		next_child->input ( command );

		// An object was able to process the command.
		if ( next_child->m_Flags.test ( xui::OBJECT_FLAG_COGITABLE ) && !cogitation )
			cogitation = true;
	};

	// Successfully distributed input.
	return cogitation;
};

// Process.
bool xui::details::input_distribution::process ( HWND hwnd , UINT msg , WPARAM wparam , LPARAM lparam ) {
	// Create new command.
	xui::input_command command ( &m_Keys );

	// Flip as a newly held key.
	static auto add_key = [ ] ( xui::input_command& command , std::uint16_t virtual_key ) {
		if ( command.m_Keys_ptr->test ( virtual_key ) )
			return;

		// Flip as a actively held key.
		command.m_Keys_ptr->flip ( virtual_key );

		//Flip as a newly pressed key.
		command.m_Keys_action.flip ( virtual_key );
	};

	// Flip as a newly released key.
	static auto remove_key = [ ] ( xui::input_command& command , std::uint16_t virtual_key ) {
		if ( !command.m_Keys_ptr->test ( virtual_key ) )
			return;

		// Flip as a actively held key.
		command.m_Keys_ptr->flip ( virtual_key );

		//Flip as a newly released key.
		command.m_Keys_action.flip ( virtual_key );
	};

	// Map msg to the corresponding virtual key and activities.
	switch ( msg ) {
	case WM_LBUTTONDOWN:
		add_key ( command , VK_LBUTTON );
		break;
	case WM_RBUTTONDOWN:
		add_key ( command , VK_RBUTTON );
		break;
	case WM_MBUTTONDOWN:
		add_key ( command , VK_MBUTTON );
		break;
	case WM_KEYDOWN:
		add_key ( command , static_cast < std::uint16_t > ( wparam ) );
		break;
	case WM_LBUTTONUP:
		remove_key ( command , VK_LBUTTON );
		break;
	case WM_RBUTTONUP:
		remove_key ( command , VK_RBUTTON );
		break;
	case WM_MBUTTONUP:
		remove_key ( command , VK_MBUTTON );
		break;
	case WM_KEYUP:
		remove_key ( command , static_cast < std::uint16_t > ( wparam ) );
		break;
		// Process mouse scrolling.
	case WM_MOUSEWHEEL:
		command.mouse.m_Scroll = -( static_cast < std::int16_t > ( HIWORD ( wparam ) ) / 120 ) * 2;
		break;
	default:
		// Update mouse location.
		if ( msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST ) {
			command.mouse.m_Location ( 
				static_cast < std::uint32_t > ( LOWORD ( lparam ) ) ,
				static_cast < std::uint32_t > ( HIWORD ( lparam ) ) 
			); break;
		};

		// Ignore and let Wndproc handle it.
		return false;
	};

	// Distribute input.
	return distribute ( command );
};
