<?php
// $Id$

/**
 * @file
 * Provides a node previous/next pager.
 */

/**
 * Define the Panels plugin.
 */
$plugin = array(
  'title' => t('Node pager'),
  'content_types' => 'node_pager_content_type',
  'single' => TRUE,
  'render callback' => 'node_pager_content_type_render',
  'icon' => 'icon_node_pager.png',
  'description' => t('Node pager content type - works with a node context.'),
  'required context' => new ctools_context_required(t('Node'), 'node'),
  'edit form' => 'node_pager_content_type_edit_form',
  'admin title' => 'node_pager_content_type_admin_title',
  'category' => array(t('Node'), 50),
);

//------------------------------------------------------------------------------

/**
 * Return the administrative title for the node pager content type.
 */
function node_pager_content_type_admin_title($subtype, $conf, $context = NULL) {
  $output = t('Node pager');
  if ($conf['override_title'] && !empty($conf['override_title_text'])) {
    $output = filter_xss_admin($conf['override_title_text']);
  }
  return $output;
}

//------------------------------------------------------------------------------

/**
 * Edit form for the node pager Panels content type.
 */
function node_pager_content_type_edit_form(&$form, &$form_state) {
  $conf     = $form_state['conf'];
  $contexts = $form_state['contexts'];

  // Add Javascript AHAH connectivity.
  drupal_add_js(drupal_get_path('module', 'node_pager') . '/node_pager.js');

  //  View selector.
  $form['view_id'] = array(
    '#id' => 'view-selector',
    '#type' => 'select',
    '#title' => t('Using the view'),
    '#default_value' => ($conf['view_id'] ? $conf['view_id'] : NULL),
    '#options' => node_pager_get_views(),
    '#description' => t('The view that you wish to use to order nodes in this node pager.'),
  );

  // Display selector.
  $form['view_display'] = array(
    '#id' => 'view-display-selector',
    '#type' => 'select',
    '#title' => t('View display'),
    '#default_value' => (
      $conf['view_id']
      ? $conf['view_id'] . ':' . $conf['view_display']
      : NULL
    ),
    '#options' => node_pager_get_view_displays(), // Assume no Javascript
    '#description' => t('The view display you select from the option above will be used to order the nodes in this node pager.'),
  );

  // Overview path.
  $form['path'] = array(
    '#type' => 'textfield',
    '#default_value' => isset($conf['path']) ? $conf['path'] : '',
    '#title' => t('Overview path'),
    '#size' => 30,
    '#description' => t('If this is set, an overview link will be available in the node pager display.'),
  );
  if (!empty($contexts)) {
    $form['path']['#description'] .= ' ' . t('You may use substitutions in this path.');

    // We have to create a manual fieldset because fieldsets do not support IDs.
    // Use 'hidden' instead of 'markup' so that the process will run.
    $form['contexts_prefix'] = array(
      '#type' => 'hidden',
      '#id' => 'edit-path-substitutions',
      '#prefix' => '<div><fieldset id="edit-path-substitutions" class="collapsed collapsible"><legend>' . t('Substitutions') . '</legend>',
    );

    $rows = array();
    foreach ($contexts as $context) {
      foreach (ctools_context_get_converters('%' . check_plain($context->keyword) . ':', $context) as $keyword => $title) {
        $rows[] = array(
          check_plain($keyword),
          t('@identifier: @title', array('@title' => $title, '@identifier' => $context->identifier)),
        );
      }
    }
    $header = array(t('Keyword'), t('Value'));
    $form['contexts']['context'] = array('#value' => theme('table', $header, $rows));
    $form['contexts_suffix'] = array(
      '#value' => '</fieldset></div>',
    );
  }
  return $form;
}

/**
 * Validate node pager edit form submission.
 *
 * If the view and the display do not line up, throw an error.
 */
function node_pager_content_type_edit_form_validate($form, &$form_state) {

  // Validate view.
  $view = views_get_view($form_state['values']['view_id']);
  list($view_id, $display_name) = explode(':', $form_state['values']['view_display']);

  $form_state['values']['view_display'] = $display_name;

  if ($view->vid != $view_id) {
    form_set_error('view_display',
      t('You must select a display from the selected view.'
        . '  You wouldn\'t have this problem if you would enable Javascript.')
    );
  }
}

/**
 * Handle node pager edit form submission.
 */
function node_pager_content_type_edit_form_submit(&$form, &$form_state) {
  foreach (array('view_id', 'view_display', 'path') as $key) {
    if (!empty($form_state['values'][$key])) {
      $form_state['conf'][$key] = $form_state['values'][$key];
    }
  }
}

//------------------------------------------------------------------------------

/**
 * Render the node pager.
 */
function node_pager_content_type_render($subtype, $conf, $args, $context) {
  if (!empty($context) && empty($context->data)) {
    return;
  }
  $node = isset($context->data) ? drupal_clone($context->data) : NULL;

  $block = new stdClass();
  $block->module = 'node';
  $block->title  = t('Pager');

  if (empty($node)) {
    $block->delta   = 'placeholder';
    $block->content = t('Node content goes here.');
  }
  else {
    $view = views_get_view($conf['view_id']);
    $view->set_display($conf['view_display']);
    $view->set_items_per_page(0); // Disable paging.  Return everything.
    $view->execute();

    $previous = array(); // unshift
    $next     = array(); // push

    foreach ($view->result as $item) {
      if ($item->nid == $node->nid) {
        $after_nid = TRUE;
      }
      elseif ($after_nid) { // After node
        array_push($next, $item->nid);
      }
      else { // Before node
        array_unshift($previous, $item->nid);
      }
    }
    $view->destroy();

    if (!empty($conf['path'])) {
      $conf['path'] = ctools_context_keyword_substitute($conf['path'], array(), array($context));
    }
    $block->delta   = $node->nid;
    $block->content = theme('node_pager', $node, $previous, $next, $conf['path']);
  }
  return $block;
}
