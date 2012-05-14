<?php

/**
 * @file
 *
 * Functions for rendering/vaildating/submitting the CSV subsection of the Autocomplet URL form.
 */

/**
 * Returns the form definition for the csv source part of the autocomplete url form.
 *
 * @param array $form_state
 *   The drupal form state.
 * @param int $url_id
 *   The id of the URL being edited, It is null when creating a new url.
 *
 * @return array
 *   The csv source part of the autocomplete url form.
 */
function islandora_autocomplete_csv_form(array &$form_state, $url_id = NULL) {
  module_load_include('inc', 'islandora_autocomplete_csv', 'includes/islandora_autocomplete_csv.db');
  return array(
    '#title' => NULL,
    '#description' => t('Upload a valid CSV file where each line has two values enclode in "quotes" and seperated by a comma. The first will be used as the value and the second is the will be used as the label present to the user.'),
    'file' => array(
      '#title' => t('CSV upload'),
      '#type' => 'file',
    )
  );
}

/**
 * Validate the source portion of the submitted form.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 */
function islandora_autocomplete_csv_form_validate(array $form, array &$form_state) {
  $validators = array('islandora_autocomplete_csv_validate_file_upload' => array('csv'));
  $file = file_save_upload('file', $validators, FILES_ISLANDORA_AUTOCOMPLETE, FILE_EXISTS_RENAME);
  if ($file !== 0) {
    file_set_status($file, FILE_STATUS_PERMANENT);
    $form_state['values']['file'] = $file->filepath;
  }
  else {
    $form_state['values']['file'] = NULL;
  }
}

/**
 * Prevent the upload of non csv files.
 *
 * @global object $user
 * @param object $file
 *   The file object to validate.
 * @param string $extensions
 *   List of supported extensions seperated by spaces.
 * @return array
 *   Errors in the format documented in file_save_upload.
 */
function islandora_autocomplete_csv_validate_file_upload($file, $extensions) {
  global $user;
  $errors = array();
  $regex = '/\.(' . @ereg_replace(' +', '|', preg_quote($extensions)) . ')$/i';
  if (!preg_match($regex, $file->filename)) {
    $errors[] = t('Only files with the following extensions are allowed: %files-allowed.', array('%files-allowed' => $extensions));
  }
  return $errors;
}

/**
 * Submits the source portion of the form.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 */
function islandora_autocomplete_csv_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_autocomplete_csv', 'includes/islandora_autocomplete_csv.db');
  extract($form_state['values'], EXTR_SKIP);
  if (!empty($file)) { // File was uploaded.
    if (isset($url_id)) { // File already existed
      $source = islandora_autocomplete_csv_db_get_url_source($url_id);
      if ($source) { // Source exists, update it.
        islandora_autocomplete_csv_db_update_url_source($source->id, $url_id, $file);
        return; // Updated Source.
      } // Add new source.
      islandora_autocomplete_csv_db_add_url_source($url_id, $file);
    }
  }
}