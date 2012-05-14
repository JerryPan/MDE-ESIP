<?php

/**
 * @file
 *
 * Contains functions for rendering and processing autocomplete_group elements.
 */

/**
 * Implements theme_element();
 */
function theme_autocomplete_group($element) {
  _form_set_class($element, array(
    'autocomplete_group'
  ));
  $id = $element['#id'];
  $attributes = drupal_attributes($element['#attributes']);
  return theme('form_element', $element, "<div id='$id'{$attributes}>{$element['#children']}</div>");
}

/**
 * Returns all the form element types that can be used as autocomplete_multifields.
 *
 * @return array
 *   All the form element types that can be used as autocomplete_multifields.
 */
function autocomplete_group_get_autocomplete_types() {
  return array('textfield');
}

/**
 * Checks to see if the given type is a autocomplete_multifield type.
 *
 * @param string $type
 *   A form element type such as 'textfield'.
 * @return boolean
 *   Checks to see if the form element type is a
 */
function autocomplete_group_is_autocomplete_multifield_type($type) {
  $types = autocomplete_group_get_autocomplete_types();
  return array_search($type, $types) !== FALSE;
}

/**
 * Sets all the required properties autocomplete_group child autocomplete_multifield elements.
 *
 * @param array $element
 *   The element whose children's properties are to be set.
 * @param string $path
 *   The #autocomplete_path property.
 * @param string $group
 *   The #id property of the parent autocomplete_group element.
 * @param string $format
 *   The default #user_data['format'] property.
 * @param int $index
 *   The index of the current autocomplete_multifield property relvant to its sibiling autocomplete_multifield properties.
 *
 * @return array
 *   The element whose children's properties are to be set.
 */
function autocomplete_group_set_child_multifield_properties(array &$element, $path, $group, $format, $index = 0) {
  $children = element_children($element);
  foreach ($children as $key) {
    $child = &$element[$key];
    if (autocomplete_group_is_autocomplete_multifield_type($child['#type'])) {
      $child = autocomplete_group_set_multifield_properties($child, $path, $group, $format, $index);
      $index++;
    }
    autocomplete_group_set_child_multifield_properties($child, $path, $group, $format, $index); // Recurse
  }
  return $element;
}

/**
 * Set's the multifield properties for the given element.
 *
 * @param array $element
 *   The form element to set the multifield properties.
 * @param string $path
 *   The #autocomplete_path property.
 * @param string $group
 *   The #id property of the parent autocomplete_group element.
 * @param string $format
 *   The default #user_data['format'] property. This may be overwritten by a user defined format.
 * @param int $index
 *   The index of the current autocomplete_multifield property relvant to its sibiling autocomplete_multifield properties.
 *   This may be overwritten by a user defined index.
 *
 * @return array
 *   The form element whose multifield properties have been set.
 */
function autocomplete_group_set_multifield_properties(array $element, $path, $group, $format, $index) {
  $format = isset($element['#user_data']['format']) ? $element['#user_data']['format'] : $format;
  $index = isset($element['#user_data']['index']) ? $element['#user_data']['index'] : $index;
  $element['#autocomplete_path'] = $path;
  $element['#user_data']['group'] = $group;
  $element['#user_data']['format'] = $format;
  $element['#user_data']['index'] = $index;
  $element['#pre_render'] = isset($element['#pre_render']) ? $element['#pre_render'] : array();
  array_unshift($element['#pre_render'], 'autocomplete_group_add_settings');
  $element['#post_render'] = isset($element['#post_render']) ? $element['#post_render'] : array();
  array_unshift($element['#post_render'], 'autocomplete_group_child_correct_class');
  return $element;
}

/**
 * Includes the formating and request data that is used by the behaviors attached to autocomplete_multifield's
 * to request/format/populate.
 *
 * @param array $element
 *   The autocomplete_multifield's form properties.
 */
function autocomplete_group_add_settings(array $element) {
  $settings = array(
    $element['#id'] => array(
      'group' => $element['#user_data']['group'],
      'index' => $element['#user_data']['index'],
      'format' => $element['#user_data']['format']
    )
  );
  drupal_add_js(array('autocomplete_group' => $settings), 'setting');
  $path = drupal_get_path('module', 'islandora_autocomplete_multifield');
  drupal_add_js("$path/js/autocomplete_multifield.js", 'module');
  return $element;
}

/**
 * Changes the class on the input field from autocomplete to autocomplete_multifield so that the right
 * Drupal behaviors get attached to autocomplete_multifields.
 *
 * @param string $output
 *   The rendered html for the element to change.
 * @param array $element
 *   The form data for the rendered element.
 *
 * @return
 *   The altered html.
 */
function autocomplete_group_child_correct_class($output, array $element) {
  return preg_replace('/class\=\"autocomplete\"/', 'class="autocomplete_multifield"', $output);
}
