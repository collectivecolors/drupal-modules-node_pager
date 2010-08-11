// $Id$

Drupal.behaviors.node_pager_initialize = function(context) {
	
	//--------------------------------------------------------------------------
	// Properties

	var current_display = $('#view-display-selector').val();

	//--------------------------------------------------------------------------
	// Handlers
	
	var set_active_option = function() {
		$('#view-display-selector').val(current_display);
	}
	
	//--------------------------------------------------------------------------
	
	var view_change_handler = function() {
		var view_id = $('#view-selector').val();
		
		if (!view_id) {
			view_id = $('#view-selector option:first').val();
			$('#view-selector').val(view_id);
		}		
		// Load new view displays.
		$('#view-display-selector').load(
			'/node_pager/js/display_options?view_id=' + view_id, 
			'', set_active_option
		);
	}
	
	//--------------------------------------------------------------------------
	// Start	
	
	// Reload displays when views are changed.
	$('#view-selector').change(view_change_handler);
	view_change_handler();
}