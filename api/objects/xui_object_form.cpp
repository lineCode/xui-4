#include <xui/xui.hpp>

// Run command against form object input.
void xui::object_form::input ( xui::input_command& command ) {
	// Flip cogitable status.
	if ( command.key_in < xui::KEY_ACTIVITY_RELEASE > ( VK_DELETE ) )
		m_Flags.flip ( xui::OBJECT_FLAG_COGITABLE );

	// Object isn't cogitable.
	if ( !m_Flags.test ( xui::OBJECT_FLAG_COGITABLE ) ||
		// Object was disabled from interactions.
		m_Flags.test ( xui::OBJECT_FLAG_DISABLED ) ) {

		// Reset hovered/interaction flag.
		m_Flags.reset ( xui::OBJECT_FLAG_HOVERED );
		m_Flags.reset ( xui::OBJECT_FLAG_INTERACTION );

		return;
	};

	// Set hovered flag.
	m_Flags.set ( xui::OBJECT_FLAG_HOVERED ,
		// Is in header region.
		command.mouse_location ( ).inside ( m_Location , { m_Size [ 0 ] , m_Header_height } ) ||
		// Is already in an interaction.
		m_Flags.test ( xui::OBJECT_FLAG_INTERACTION ) );

	// Mouse initial press in header area.
	if ( command.key_in < xui::KEY_ACTIVITY_PRESS > ( VK_LBUTTON ) &&
		m_Flags.test ( xui::OBJECT_FLAG_HOVERED ) && 
		!m_Flags.test ( xui::OBJECT_FLAG_INTERACTION ) ) {
		// Set current mouse location.
		m_Previous_mouse_location = command.mouse_location ( );

		// Focus self to Api.
		self_focus ( m_Api_ptr );

		// Enable interaction flag
		m_Flags.set ( xui::OBJECT_FLAG_INTERACTION );
	};

	// We've lost focus.
	if ( command.key_in < xui::KEY_ACTIVITY_RELEASE > ( VK_LBUTTON ) &&
		m_Flags.test ( xui::OBJECT_FLAG_INTERACTION ) ) {
		// Set last mouse location.
		m_Previous_mouse_location = command.mouse_location ( );

		// Unfocus self from Api.
		self_unfocus ( m_Api_ptr );

		// Remove interaction flag.
		m_Flags.reset ( xui::OBJECT_FLAG_INTERACTION );
	};

	// Update location drag.
	if ( m_Flags.test ( xui::OBJECT_FLAG_INTERACTION ) ) {
		auto next_location { m_Location };

		// Get the delta between previous/current location.
		next_location += command.mouse_location ( );
		next_location -= m_Previous_mouse_location;

		// Setup for next location.
		m_Location = next_location;

		// Store current mouse location.
		m_Previous_mouse_location = command.mouse_location ( );
	};

	// No children.
	if ( m_Children_ptrs.empty ( ) )
		return;

	// Pass command to children.
	for ( auto& next_child : m_Children_ptrs ) {
		if ( !next_child->m_Api_ptr )
			next_child->m_Api_ptr = xui::g_Api.get ( );

		if ( !next_child->m_Parent_ptr )
			next_child->m_Parent_ptr = this;

		// Disable object while another is enabled.
		if ( m_Focused_ptr && m_Focused_ptr != next_child.get ( ) &&
			!next_child->m_Flags.test ( xui::OBJECT_FLAG_DISABLED ) )
			next_child->m_Flags.flip ( xui::OBJECT_FLAG_DISABLED );

		// Re-enable objects that weren't focused.
		if ( !m_Focused_ptr && next_child->m_Flags.test ( xui::OBJECT_FLAG_DISABLED ) )
			next_child->m_Flags.flip ( xui::OBJECT_FLAG_DISABLED );

		// Run objects input against command.
		next_child->input ( command );
	};
};

// Render form object.
void xui::object_form::render ( void ) {
	/*
	
	...Render...

	*/

	if ( m_Children_ptrs.empty ( ) )
		return;

	for ( auto iter = m_Children_ptrs.rbegin ( ); iter != m_Children_ptrs.rend ( ); ++iter )
		iter->get ( )->render ( );
};