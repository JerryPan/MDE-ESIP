<?php

/**
 * @file
 *
 * Functions to abstract this modules database usage.
 */
define('ISLANDORA_AUTOCOMPLETE_URL_MAX_NAME_LENGTH', 64);
define('ISLANDORA_AUTOCOMPLETE_URL_MAX_DESCRIPTION_LENGTH', 256);
define('ISLANDORA_AUTOCOMPLETE_URL_MAX_SOURCE_NAME_LENGTH', 64);
define('TABLE_ISLANDORA_AUTOCOMPLETE', 'islandora_autocomplete');

/**
 * Performs a query and retrieves all the Autocomplete URL's stored in the database.
 *
 * @return
 *   A database query result resource, or FALSE if the query was not
 *   executed correctly.
 */
function islandora_autocomplete_db_get_urls() {
  return db_query('SELECT * FROM {%s}', TABLE_ISLANDORA_AUTOCOMPLETE);
}

/**
 * Performs a query and retrieves all the Autocomplete URL's stored in the database that are not used with the
 * multifield form.
 *
 * @return
 *   A database query result resource, or FALSE if the query was not
 *   executed correctly.
 */
function islandora_autocomplete_db_get_non_multifield_urls() {
  return db_query('SELECT * FROM {%s} WHERE fields = 1', TABLE_ISLANDORA_AUTOCOMPLETE);
}

/**
 * Fetches the given Autocomplete URL from the database.
 *
 * @param int $id
 *   The unique id for the requested Autocomplete URL.
 *
 * @return
 *   An object representing the given autocomplete url identified by name, or FALSE. The attributes
 *   of this object are the table fields selected by the query.
 */
function islandora_autocomplete_db_get_url($id) {
  $result = db_query('SELECT * FROM {%s} WHERE id = %d', TABLE_ISLANDORA_AUTOCOMPLETE, $id);
  return $result !== FALSE ? db_fetch_object($result) : FALSE;
}

/**
 * Fetches the given Autocomplete URL from the database.
 *
 * @param string $name
 *   The unique machine name for the requested Autocomplete URL.
 *
 * @return
 *   An object representing the given autocomplete url identified by name, or FALSE. The attributes
 *   of this object are the table fields selected by the query.
 */
function islandora_autocomplete_db_get_url_by_name($name) {
  $result = db_query('SELECT * FROM {%s} WHERE name = "%s"', TABLE_ISLANDORA_AUTOCOMPLETE, $name);
  return $result !== FALSE ? db_fetch_object($result) : FALSE;
}

/**
 * Adds a new Autocomplete URL if one by the same name doesn't already exist.
 *
 * @param string $name
 *   The unique name of the URL to add.
 * @param string $description
 *   A short description of the URL's purpose.
 * @param string $source
 *   The source used to populate the autocomplete URL.
 * @param string $fields
 *   The number of fields within the source.
 *
 * @return boolean
 *   TRUE if successful FALSE otherwise.
 */
function islandora_autocomplete_db_add_url($name, $description, $source, $fields) {
  if (db_result(db_query('SELECT count(id) FROM {%s} WHERE name = "%s"', TABLE_ISLANDORA_AUTOCOMPLETE, $name)) == 0) {
    $object = new stdClass();
    $object->name = $name;
    $object->description = $description;
    $object->source = $source;
    $object->fields = $fields;
    return drupal_write_record(TABLE_ISLANDORA_AUTOCOMPLETE, $object) !== FALSE;
  }
  return FALSE;
}

/**
 * Update the given Autocomplete URL.
 *
 * @param int $id
 *   The unique ID for the record we want to update.
 * @param string $name
 *   The unique name of the URL to add.
 * @param string $description
 *   A short description of the URL's purpose.
 * @param string $source
 *   The source used to populate the autocomplete URL.
 * @param string $fields
 *   The number of fields within the source.
 *
 * @return boolean
 *   TRUE if successful FALSE otherwise.
 */
function islandora_autocomplete_db_update_url($id, $name, $description, $source, $fields) {
  if (db_result(db_query('SELECT count(id) FROM {%s} WHERE id = %d', TABLE_ISLANDORA_AUTOCOMPLETE, $id)) == 1) {
    $object = new stdClass();
    $object->id = $id;
    $object->name = $name;
    $object->description = $description;
    $object->source = $source;
    $object->fields = $fields;
    return drupal_write_record(TABLE_ISLANDORA_AUTOCOMPLETE, $object, 'id');
  }
  return FALSE;
}

/**
 * Deletes the given Autocomplete URL.
 *
 * @param int $id
 *   The unique id of the URL to delete.
 */
function islandora_autocomplete_db_delete_url($id) {
  module_invoke_all('delete_autocomplete_url', $id);
  db_query('DELETE FROM {%s} WHERE id = "%d"', TABLE_ISLANDORA_AUTOCOMPLETE, $id);
}

/**
 * Adds a foriegn key constaint to the given table such that if a given autocomplete url is
 * delete than the corresponding row in the given table will be deleted as well.
 *
 * Note:
 *  This requires the the given table has a field url_id which is to be used as the
 *  foriegn key for the given table.
 *
 * @param string $table
 *   The name of the table that requires the foreign key constraint.
 */
function islandora_autocomplete_add_foreign_key_constraints_for($table) {
  // Only InnoDB supports foreign keys. Other formats fail silently.
  db_query("ALTER TABLE `{$table}` ENGINE = InnoDB");
  $constraint_query = <<<EOT
  ALTER TABLE `{$table}` ADD CONSTRAINT `{$table}_url_id`
  FOREIGN KEY (`url_id`)
  REFERENCES `{%table%}` (`id`)
  ON DELETE CASCADE
  ON UPDATE NO ACTION
EOT;
  $query = str_replace('%table%', TABLE_ISLANDORA_AUTOCOMPLETE, $constraint_query);
  db_query($query);
}