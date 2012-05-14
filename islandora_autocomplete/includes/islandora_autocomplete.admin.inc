<?php

/**
 * @file
 *
 * Functions for administering Autocomplete url's that allow the users to
 * create, update, read, delete Autocomplete URLs.
 */
module_load_include('inc', 'islandora_autocomplete', 'includes/islandora_autocomplete.db');

/**
 * Menu callback; admin menu for autocomplete url's.
 *
 * @return string
 *   The admin page's HTML.
 */
function islandora_autocomplete_admin() {
  $multifield_exists = module_exists('islandora_autocomplete_multifield');
  if ($multifield_exists) {
    $header = array(t('Name'), t('URL'), t('Description'), t('Source'), t('#Fields'), array('data' => t('Operations'), 'colspan' => '3'));
    $urls = islandora_autocomplete_db_get_urls();
  }
  else {
    $header = array(t('Name'), t('URL'), t('Description'), t('Source'), array('data' => t('Operations'), 'colspan' => '3'));
    $urls = islandora_autocomplete_db_get_non_multifield_urls();
  }
  $rows = array();
  while ($url = db_fetch_object($urls)) {
    $path = str_replace('%', $url->name, MENU_ISLANDORA_AUTOCOMPLETE_URL);
    $view_path = str_replace('%', $url->name, MENU_ISLANDORA_AUTOCOMPLETE_VIEW_URL);
    $edit_path = str_replace('%', $url->name, MENU_ISLANDORA_AUTOCOMPLETE_EDIT_URL);
    $delete_path = str_replace('%', $url->name, MENU_ISLANDORA_AUTOCOMPLETE_DELETE_URL);
    $row = array(
      l($url->name, $path),
      MENU_ISLANDORA_AUTOCOMPLETE . '/' . $url->name,
      filter_xss_admin($url->description),
      check_plain($url->source),
    );
    if ($multifield_exists) {
      $row[] = $url->fields;
    }
    $row[] = array('data' => l(t('view'), $view_path));
    $row[] = array('data' => l(t('edit'), $edit_path));
    $row[] = array('data' => l(t('delete'), $delete_path));
    $rows[] = $row;
  }
  if (empty($rows)) {
    $rows[] = array(array('data' => t('No autocomplete url\'s available.'), 'colspan' => '5', 'class' => 'message'));
  }
  return theme('table', $header, $rows);
}

/**
 * Displays the content of the view url.
 *
 * @param string $name
 *   The name of Autocomplete URL to display.
 */
function islandora_autocomplete_view_form(array &$form_state, $name) {
  module_load_include('inc', 'islandora_autocomplete', 'includes/autocomplete');
  $values = islandora_autocomplete_view_form_values($form_state, $name);
  $header = islandora_autocomplete_view_form_table_header($values);
  $rows = islandora_autocomplete_view_form_table_rows($values);
  return array(
    'name' => array(
      '#type' => 'hidden',
      '#value' => $values['name']
    ),
    'count' => array(
      '#type' => 'hidden',
      '#value' => $values['count']
    ),
    'query' => array(
      '#title' => t('Query'),
      '#type' => 'textfield',
      '#default_value' => $values['query'],
    ),
    'field' => (module_exists('islandora_autocomplete_multifield') && $values['count'] > 1) ? array(
      '#title' => t('Field'),
      '#type' => 'select',
      '#options' => islandora_autocomplete_view_form_field_options($values['count']),
      '#default_value' => $values['field']) : NULL,
    'submit' => array(
      '#type' => 'submit',
      '#value' => 'Test'
    ),
    'cancel' => array(
      '#type' => 'markup',
      '#value' => l(t('Cancel'), MENU_ISLANDORA_AUTOCOMPLETE_ADMIN)
    ),
    'results' => array(
      '#title' => t('Query Results'),
      '#type' => 'fieldset',
      'table' => array(
        '#type' => 'markup',
        '#value' => theme('table', $header, $rows)
      )
    )
  );
}

/**
 *
 * @param array $form_state
 *   The drupal form state.
 * @param string $name
 *   The name of Autocomplete URL to view.
 */
function islandora_autocomplete_view_form_values(array &$form_state, $name) {
  $url = islandora_autocomplete_db_get_url_by_name($name);
  $values = isset($form_state['values']) ? $form_state['values'] : array();
  $values['name'] = $name;
  $values['count'] = $url->fields;
  $values['query'] = isset($values['query']) ? $values['query'] : '';
  if ($url->fields > 1) {
    $values['field'] = isset($values['field']) ? $values['field'] : 1;
  }
  return $values;
}

/**
 * Gets the header for the view form table.
 *
 * @param array $values
 *   The values for the form fields.
 * @return array
 *   The header for the view form table.
 */
function islandora_autocomplete_view_form_table_header($values) {
  $header = array();
  for ($i = 0; $i < $values['count']; $i++) {
    $args = array('!i' => $i + 1);
    $header = array_merge($header, array(t('Field !i: Value', $args), t('Field !i: Label', $args)));
  }
  return $header;
}

/**
 * Generate the rows for the view table form.
 *
 * @param array $values
 *   The values for the form fields.
 * @return array
 *   The rows for the view form table.
 */
function islandora_autocomplete_view_form_table_rows($values) {
  $url = islandora_autocomplete_db_get_url_by_name($values['name']);
  $index = isset($values['field']) ? $values['field'] - 1 : NULL;
  $results = islandora_autocomplete_query_source($url->id, $url->source, $values['query'], $index); // Count the offset
  $rows = array();
  if ($url->fields == 1) {
    foreach ($results as $key => $value) {
      $rows[] = array($key, $value);
    }
  }
  else {
    foreach ($results as $result) {
      $row = array();
      array_walk_recursive($result, create_function('&$v, $k, &$o', '$o[] = $v;'), &$row);
      $rows[] = $row;
    }
  }
  if (empty($rows)) {
    $rows[] = array(array('data' => t('No results.'), 'colspan' => (string) 2 * $url->fields, 'class' => 'message'));
  }
  return $rows;
}

/**
 * Gets the source options for view form.
 *
 * @return array
 *   The source options for a select field.
 */
function islandora_autocomplete_view_form_field_options($count) {
  $options = array();
  for ($i = 1; $i < $count + 1; $i++) {
    $options[$i] = $i;
  }
  return $options;
}

/**
 * Rebuilds the View form.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 */
function islandora_autocomplete_view_form_submit(array $form, array &$form_state) {
  $form_state['rebuild'] = TRUE;
}

/**
 * Form definition for adding/updating and autocomplete url.
 *
 * @param array $form_state
 *   The drupal form state.
 * @param string $name
 *   The name of Autocomplete URL to update. If not preset this form will be used to create a new url.
 */
function islandora_autocomplete_form(array &$form_state, $name = '') {
  islandora_autocomplete_form_include_resources();
  $values = islandora_autocomplete_form_values($form_state, $name);
  if (empty($values['source'])) {
    drupal_set_message(t('No sources are defined, please install a module like "islandora autocomplete csv" so that a source can be defined.'), 'error');
    return array();
  }
  return array(
    '#action' => empty($name) ? MENU_ISLANDORA_AUTOCOMPLETE_ADD_URL : str_replace('%', $name, MENU_ISLANDORA_AUTOCOMPLETE_URL),
    '#attributes' => array(
      'enctype' => "multipart/form-data"
    ),
    'id' => array(
      '#type' => 'hidden',
      '#value' => $values['id'],
    ),
    'url_id' => array(// Copy of id, to simplify the source forms handling of this value.
      '#type' => 'hidden',
      '#value' => $values['url_id'],
    ),
    'name' => array(
      '#title' => t('Name'),
      '#description' => t('The machine-readable name of this autocomplete url. This text will be used for constructing the URL of #autocomplete_path. This name must contain only lowercase letters, numbers, and underscores. This name must be unique.'),
      '#type' => 'textfield',
      '#required' => TRUE,
      '#default_value' => $values['name'],
      '#element_validate' => array('islandora_autocomplete_form_validate_name'),
    ),
    'description' => array(
      '#title' => t('Description'),
      '#description' => t('A brief description of this content type.'),
      '#required' => TRUE,
      '#default_value' => $values['description'],
      '#type' => 'textarea',
    ),
    'fields' => module_exists('islandora_autocomplete_multifield') ? array(
      '#title' => t('Fields'),
      '#type' => 'select',
      '#description' => t('The number of fields this url will return, there should be the same number of fields in the form to recieve the values.'),
      '#options' => array(1 => 1, 2 => 2, 3 => 3, 4 => 4, 5 => 5, 6 => 6, 7 => 7, 8 => 8, 9 => 9, 10 => 10),
      '#default_value' => $values['fields']) : NULL,
    'source' => array(
      '#title' => t('Source'),
      '#name' => 'source',
      '#description' => t('This is the source from which the autocomplete url will be populated.'),
      '#type' => 'select',
      '#default_value' => $values['source'],
      '#options' => islandora_autocomplete_source_form_options(),
      '#ahah' => array(
        'path' => MENU_ISLANDORA_AUTOCOMPLETE_AHAH_SOURCE,
        'wrapper' => 'source-form-wrapper',
        'method' => 'replace',
        'effect' => 'fade',
      ),
    ),
    'source_form' => islandora_autocomplete_source_form($form_state, $values['url_id'], $values['source']),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Submit')
    ),
    'cancel' => array(
      '#value' => l(t('Cancel'), MENU_ISLANDORA_AUTOCOMPLETE_ADMIN)
    ),
  );
}

/**
 * Attempts to populate the forms values with values stored in the database
 * if they have not already been altered in the form
 *
 * @param array $form_state
 *   The drupal form state.
 * @param string $name
 *   The name of Autocomplete URL to update. If not preset this form will be used to create a new url.
 */
function islandora_autocomplete_form_values(array &$form_state, $name = '') {
  $url = !empty($name) ? object_as_array(islandora_autocomplete_db_get_url_by_name($name)) : FALSE;
  $default_source = islandora_autocomplete_get_default_source();
  $values = isset($form_state['values']) ? $form_state['values'] : $url;
  $values['id'] = isset($values['id']) ? $values['id'] : NULL;
  $values['url_id'] = isset($values['url_id']) ? $values['url_id'] : $values['id'];
  $values['name'] = isset($values['name']) ? $values['name'] : '';
  $values['description'] = isset($values['description']) ? $values['description'] : '';
  $values['fields'] = isset($values['fields']) ? $values['fields'] : 1;
  $values['source'] = isset($values['source']) ? $values['source'] : $default_source['type'];
  return $values;
}

/**
 * Includes the required js/css resources needed for this form.
 */
function islandora_autocomplete_form_include_resources() {
  $path = drupal_get_path('module', 'islandora_autocomplete');
  drupal_add_js("$path/js/fix-jquery-update.js", 'theme'); // Hacktacular, fixes issues between jquery update file uploads and AHAH.
}

/**
 * Gets the form definition for the given source type as defined by hook_register_autocomplete_source().
 *
 * @param array $form_state
 *   The drupal form state.
 * @param string $url_id
 *   The id of the url whose source form will be returned. May be NULL when creating a new URL.
 * @param string $type
 *   The 'type' value for the registered autocomplete source.
 *
 * @return array
 *   The source form definition.
 */
function islandora_autocomplete_source_form(array &$form_state, $url_id, $type) {
  $wrapper = array(
    '#title' => t('Source Settings'),
    '#type' => 'item',
    '#prefix' => '<div id="source-form-wrapper">',
    '#suffix' => '</div>'
  );
  $form = array();
  if (($source = islandora_autocomplete_get_source($type)) != FALSE) {
    module_load_include($source['form']['type'], $source['module'], $source['form']['file']);
    $hook = "{$source['module']}_form";
    $form = function_exists($hook) ? call_user_func($hook, $form_state, $url_id) : array();
  }
  return array_merge($wrapper, $form);
}

/**
 * Gets the source options.
 *
 * @return array
 *   The source options for a select field.
 */
function islandora_autocomplete_source_form_options() {
  static $options = array();
  if (empty($options)) {
    $sources = islandora_autocomplete_get_sources();
    foreach ($sources as $source) {
      $type = $source['type'];
      $label = $source['label'];
      $options[$type] = $label;
    }
  }
  return $options;
}

/**
 * Validates the 'name' form element, ensures the proper format and that the given name is unique.
 *
 * @param array $element
 *   The form element to validate.
 * @param array $form_state
 *   The drupal form state.
 * @param array $complete_form
 *   The drupal form.
 */
function islandora_autocomplete_form_validate_name($element, array &$form_state) {
  $values = $form_state['values'];
  $name = $values['name'];
  $original_url = isset($values['id']) ? islandora_autocomplete_db_get_url($values['id']) : FALSE;
  $check_for_uniqueness = $original_url ? $original_url->name != $name : TRUE;
  if (preg_match('/^[a-z0-9_]*$/', $name) != 1) {
    form_error($element, t('The name must contain only lowercase letters, numbers, and underscores.'));
  }
  elseif ($check_for_uniqueness && islandora_autocomplete_db_get_url_by_name($name) !== FALSE) {
    form_error($element, t('The name must be unique.'));
  }
}

/**
 * Validate the submitted form.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 */
function islandora_autocomplete_form_validate(array $form, array &$form_state) {
  $type = $form_state['values']['source'];
  if (($source = islandora_autocomplete_get_source($type)) != FALSE) {
    module_load_include($source['form']['type'], $source['module'], $source['form']['file']);
    $hook = "{$source['module']}_form_validate";
    if (function_exists($hook)) {
      call_user_func_array($hook, array($form, &$form_state));
    }
  }
}

/**
 * Either adds a autocomplete url or updates and existing one.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 */
function islandora_autocomplete_form_submit(array $form, array &$form_state) {
  extract($form_state['values'], EXTR_SKIP);
  if (isset($id)) {
    islandora_autocomplete_form_submit_update_source($id, $source);
    islandora_autocomplete_db_update_url($id, $name, $description, $source, $fields);
  }
  else {
    if (islandora_autocomplete_db_add_url($name, $description, $source, $fields) !== FALSE) {
      $url = islandora_autocomplete_db_get_url_by_name($name);
      $values = &$form_state['values'];
      $values['id'] = $values['url_id'] = $url->id; // For use in the source submit handler.
    }
  }
  islandora_autocomplete_source_form_submit($source, $form, $form_state);
  $form_state['redirect'] = MENU_ISLANDORA_AUTOCOMPLETE_ADMIN;
}

/**
 * Updates the source of the given url, if its source has changed,
 * remove the data associated with its old source.
 *
 * @param int $id
 *   The autocomplete url ID.
 * @param string $source
 *   The new source of the autocomplete url.
 */
function islandora_autocomplete_form_submit_update_source($id, $source) {
  $original_url = islandora_autocomplete_db_get_url($id);
  if ($original_url->source != $source) { // Remove the old source
    module_invoke_all('delete_autocomplete_url', $id);
  }
}

/**
 * Call the submit hook for the source section of the form.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 */
function islandora_autocomplete_source_form_submit($type, array $form, array &$form_state) {
  if (($source = islandora_autocomplete_get_source($type)) != FALSE) {
    module_load_include($source['form']['type'], $source['module'], $source['form']['file']);
    $hook = "{$source['module']}_form_submit";
    if (function_exists($hook)) {
      call_user_func_array($hook, array($form, &$form_state));
    }
  }
}

/**
 * Menu callback; delete a single autocomplete url.
 */
function islandora_autocomplete_delete_confirm(&$form_state, $name) {
  $url = islandora_autocomplete_db_get_url_by_name($name);
  if (!$url) {
    drupal_not_found();
    exit();
  }
  $form['name'] = array('#type' => 'value', '#value' => $name);
  $form['id'] = array('#type' => 'value', '#value' => $url->id);
  $message = t('Are you sure you want to delete the autocomplete url %name?', array('%name' => $name));
  $caption = '<p>' . t('This action cannot be undone.') . '</p>';
  return confirm_form($form, filter_xss($message), MENU_ISLANDORA_AUTOCOMPLETE_ADMIN, filter_xss($caption), t('Delete'));
}

/**
 * Process autocomplete url delete confirm submissions.
 */
function islandora_autocomplete_delete_confirm_submit($form, &$form_state) {
  islandora_autocomplete_db_delete_url($form_state['values']['id']);
  $t_args = array('%name' => $form_state['values']['name']);
  drupal_set_message(t('The autocomplete url %name has been deleted.', $t_args));
  watchdog('menu', 'Deleted autocomplete url %name.', $t_args, WATCHDOG_NOTICE);
  $form_state['redirect'] = MENU_ISLANDORA_AUTOCOMPLETE_ADMIN;
  return;
}

/**
 * Replace the source_form portion of the form when the source changes.
 */
function islandora_autocomplete_form_ahah() {
  if (!isset($_REQUEST['form_build_id'])) { // Request is not valid
    header("HTTP/1.0 200 OK", FALSE, 200);
    exit();
  }
  $form_build_id = $_REQUEST['form_build_id'];
  $form_state = array('storage' => NULL, 'submitted' => FALSE, 'post' => $_POST);
  if (!$form = form_get_cache($form_build_id, $form_state)) {
    header("HTTP/1.0 200 OK", FALSE, 200);
    exit();
  }
  $args = $form['#parameters'];
  $form_id = array_shift($args);
  $form_state['post'] = $form['#post'] = $_POST;
  $form['#programmed'] = $form['#redirect'] = FALSE;
  drupal_process_form($form_id, $form, $form_state);
  $form = drupal_rebuild_form($form_id, $form_state, $args, $form_build_id);
  drupal_get_messages('error'); // Clear errors.
  $html = '';
  if (isset($form)) {
    unset($form['source_form']['#prefix'], $form['source_form']['#suffix']);
    $html = drupal_render($form['source_form']);
  }
  $javascript = drupal_add_js(NULL, NULL, 'header');
  $settings = call_user_func_array('array_merge_recursive', $javascript['setting']);
  unset($settings['ahah']['']);
  drupal_json(array(
    'status' => TRUE,
    'data' => theme('status') . $html,
    'settings' => $settings,
  ));
}