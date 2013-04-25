<?php
/**
 * @file
 * General helper methods for Availability Calendar
 * - database access
 *
 * @author Erwin Derksen (http://drupal.org/user/750928)
 */

/**
 * Adds the necessary javascript.
 *
 * Adds the necessary base javascript files, settings and initialization so
 * calendars an other client side features work correctly.
 */
function availability_calendar_add_base_js() {
  static $added = FALSE;

  if (!$added) {
    $added = TRUE;

    // Add date picker library (used for date formatting)
    drupal_add_library('system', 'ui.datepicker');

    // Add the base client side API.
    drupal_add_js(drupal_get_path('module', 'availability_calendar') . '/availability_calendar.js',
      array('group' => JS_LIBRARY, 'weight' => 100));

    // Initialize the global calendar settings: states and date format.
    drupal_add_js(array(
      'availabilityCalendar' => array(
        'states' => availability_calendar_get_states('JavaScript'),
        'displayDateFormat' => availability_calendar_get_date_format_for_js(),
      )),
      array('type' => 'setting', 'group' => JS_LIBRARY));
  }
}

/**
 * Adds the necessary base javascript files, settings and initialization for the
 * given calendar.
 *
 * @param int|string $cid
 *   Existing cid (int) or temporary cid for new calendars (string).
 * @param string $allocation_type
 */
function availability_calendar_add_calendar_js($cid, $allocation_type) {
  static $added = array();
  if (!isset($added[$cid])) {
    $added[$cid] = TRUE;
    availability_calendar_add_base_js();

    drupal_add_js(array(
        'availabilityCalendar' => array(
          'calendars' => array(
            $cid => array(
              'cid' => $cid,
              'overnight' => $allocation_type === ALLOCATION_TYPE_OVERNIGHT,
        )))),
      array('type' => 'setting'));
  }
}

/**
 * Adds the necessary javascript files, settings and initialization for the
 * given calendar view.
 *
 * @param int $cvid
 * @param int|string $cid
 *   Existing cid (int) or temporary cid for new calendars (string).
 * @param string $name
 * @param array $settings
 */
function availability_calendar_add_calendar_view_js($cvid, $cid, $name, $settings) {
  static $added = FALSE;
  availability_calendar_add_calendar_js($cid, $settings['allocation_type']);

  if (!$added) {
    $added = TRUE;
    drupal_add_js(drupal_get_path('module', 'availability_calendar') . '/availability_calendar.view.js',
      array('weight' => -1));
  }

  drupal_add_js(array(
      'availabilityCalendar' => array(
        'views' => array(
          $cvid => array(
            'cvid' => $cvid,
            'cid' => $cid,
            'name' => $name,
            'splitDay' => (bool) $settings['show_split_day'],
            'selectMode' => isset($settings['selectable']) ? ($settings['selectable'] ? 'available' : 'none') : 'all',
    )))), array('type' => 'setting'));
}

/**
 * @return int
 *   A unique identifier that can be used to identify calendar views.
 */
function availability_calendar_get_cvid() {
  static $calendar_view_count = 0;

  return ++$calendar_view_count;
}

/**
 * Converts a PHP date format to a jQuery date picker format.
 *
 * @param string $format
 *   The date format to convert. If not given or empty the (possibly localized)
 *   date format for the availability_calendar_user_date date type is
 *   converted.
 *
 * @return string
 */
function availability_calendar_get_date_format_for_js($format = '') {
  $format_conversions = array(
    'j' => 'd',
    'd' => 'dd',
    'z' => 'o',
    'D' => 'D',
    'l' => 'DD',
    'n' => 'm',
    'm' => 'mm',
    'M' => 'M',
    'F' => 'MM',
    'y' => 'y',
    'Y' => 'yy',
    'U' => '@',
    "'" => "''",
  );
  $needs_js_escape = array(
    'o',
    'd',
    'D',
    'm',
    'M',
    "y",
    '@',
    '!',
  );
  if (empty($format)) {
    $format = variable_get("date_format_availability_calendar_date_display", '');
  }
  $js_format = '';
  $is_escaped = FALSE;
  for ($i = 0; $i < strlen($format); $i++) {
    $char = $format[$i];
    if ($is_escaped || !array_key_exists($char, $format_conversions)) {
      if (!$is_escaped && $char === '\\') {
        // Next character is escaped. Skip this \.
        $is_escaped = TRUE;
      }
      else {
        // We are either escaping this character or it is not a format
        // character. In both cases we have to add this character as is, though
        // it may be a character that needs escaping in the js format.
        $js_format .= in_array($char, $needs_js_escape) ? "'$char'" : $char;
        $is_escaped = FALSE;
      }
    }
    else {
      // This is a non escaped to be converted character: replace the PHP format
      // with the js format.
      $js_format .= $format_conversions[$char];
    }
  }
  if ($is_escaped) {
    // A \ at the end of the PHP format, add it to the js format as well.
    $js_format .= '\\';
  }
  return $js_format;
}

/**
 * Parses a date string according to the - possibly localized -
 * 'Availability Calendar date display' date type.
 *
 * @param string $date
 *
 * @return DateTime|false
 *   A DateTime object if the date string could successfully be parsed,
 *   false otherwise.
 */
function availability_calendar_parse_display_date($date) {
  $date_type = 'availability_calendar_date_display';
  $format = variable_get("date_format_$date_type", '');
  // Date API works in PHP5.2, DateTime::createFromFormat in PHP >= 5.3.
  return module_exists('date_api') ? new DateObject($date, NULL, $format) : DateTime::createFromFormat($format, $date);
}

/**
 * Parses a date string according to the - possibly localized -
 * 'Availability Calendar date entry' date type.
 *
 * Note that when the date popup module is enabled dates may be passed in the
 * ISO date format.
 *
 * @param string $date
 *
 * @return DateTime|false
 *   A DateTime object if the date string could successfully be parsed,
 *   false otherwise.
 */
function availability_calendar_parse_entry_date($date) {
  $date_type = 'availability_calendar_date_entry';
  $format = variable_get("date_format_$date_type", '');
  // Date API works in PHP5.2, DateTime::createFromFormat in PHP >= 5.3.
  $result =  module_exists('date_api') ? new DateObject($date, NULL, $format) : DateTime::createFromFormat($format, $date);
  if ($result instanceof DateObject && !empty($result->errors)) {
    $result = FALSE;
  }
  if ($result === FALSE && module_exists('date_popup')) {
    $result = availability_calendar_parse_iso_date($date);
  }
  return $result;
}

/**
 * Parses a date string according to the ISO date format.
 *
 * @param string $date
 *
 * @return DateTime|false
 *   A DateTime object if the date string could successfully be parsed,
 *   false otherwise.
 */
function availability_calendar_parse_iso_date($date) {
  $format = 'Y-m-d';
  // Date API works in PHP5.2, DateTime::createFromFormat in PHP >= 5.3.
  $result =  module_exists('date_api') ? new DateObject($date, NULL, AC_ISODATE) : DateTime::createFromFormat(AC_ISODATE, $date);
  if ($result instanceof DateObject && !empty($result->errors)) {
    $result = FALSE;
  }
  return $result;
}

/**
 * Formats a date according to the - possibly localized -
 * 'Availability Calendar date display' date type.
 *
 * @param DateTime $date
 *
 * @return string
 */
function availability_calendar_format_display_date($date) {
  $date_type = 'availability_calendar_date_display';
  return format_date($date->getTimestamp(), $date_type);
}

/**
 * Formats a date according to the - possibly localized -
 * 'Availability Calendar date entry' date type.
 *
 * @param DateTime $date
 *
 * @return string
 */
function availability_calendar_format_entry_date($date) {
  $date_type = 'availability_calendar_date_entry';
  return format_date($date->getTimestamp(), $date_type);
}


/*
 * DATABASE ACCESS FUNCTIONS
 * -------------------------
 */

/**
 * Returns an array of records (or labels) of states keyed by sid.
 *
 * The results can be processed or filtered based on arguments passed in:
 * - bool: filter on is_available
 * - string: process array based on the value of the string:
 *     'label': only return label, not a record array
 *     'JavaScript': convert to JavaScript syntax: real booleans and camelCase
 * - array: filter on sid (array keys should be a list of the allowed sid's)
 * Multiple filters/processors can be passed.
 *
 * @param boolean|string ...
 *   Processing or filtering to be done on the list of states.
 *
 * @return array
 *   Array of records keyed by the sid.
 */
function availability_calendar_get_states() {
  $states = &drupal_static(__FUNCTION__);
  if ($states === NULL) {
    $states = db_select('availability_calendar_state')
      ->fields('availability_calendar_state')
      ->orderBy('weight')
      ->execute()
      ->fetchAllAssoc('sid', PDO::FETCH_ASSOC);
  }
  $result = $states;
  foreach (func_get_args() as $arg) {
    if (is_bool($arg)) {
      // filter out non-available states
      $result = array_filter($result, $arg ? 'availability_calendar_filter_available' : 'availability_calendar_filter_non_available');
    }
    else if (is_array($arg)) {
      // Filter out non-allowed states. If no states are passed in, all states
      // are allowed.
      if (!empty($arg)) {
        $result = array_intersect_key($result, $arg);
      }
    }
    else {
      array_walk($result, 'availability_calendar_convert_state', $arg);
    }
  }
  return $result;
}

/**
 * array_filter() callback to filter states based on whether they are
 * to be seen as available.
 *
 * @param array $state
 *   Array containing a state.
 *
 * @return bool
 */
function availability_calendar_filter_available($state) {
  return (bool) $state['is_available'];
}

/**
 * array_filter() callback to filter states based on whether they are
 * to be seen as not available.
 *
 * @param array $state
 *   Array containing a state.
 * @return bool
 */
function availability_calendar_filter_non_available($state) {
  return !(bool) $state['is_available'];
}

/**
 * array_filter callback to convert all states retrieved from the database.
 *
 * @param array $state
 *   Array containing a state.
 * @param string $key
 * @param string $op
 */
function availability_calendar_convert_state(&$state, $key, $op) {
  if ($op === 'JavaScript') {
    $state['is_available'] = $state['is_available'] == 1;
    $state = array(
      'sid' => (int) $state['sid'],
      'cssClass' => $state['css_class'],
      'isAvailable' => $state['is_available'] == 1,
    );
  }
  else if ($op === 'label') {
    $state = t($state['label']);
  }
}

/**
 * Updates the set of states.
 *
 * @param array $states
 *   Array with the new state records (sid, css_class, label, weight,
 *   and is_available values).
 */
function availability_calendar_update_states($states) {
  $table_name = 'availability_calendar_state';
  $existing_states = availability_calendar_get_states();
  foreach ($existing_states as $sid => $existing_state) {
    $delete = TRUE;
    foreach ($states as $state) {
      if (isset($state['sid']) && $state['sid'] == $sid) {
        $delete = FALSE;
        break;
      }
    }
    if ($delete) {
      // Cascading delete: delete availability referring to this sid.
      db_delete('availability_calendar_availability')
        ->condition('sid', $sid)
        ->execute();
      // Delete state itself.
      db_delete($table_name)
        ->condition('sid', $sid)
        ->execute();
      // @todo: update field instances (warning if something changes?)
      // this omission leads to a: Warning: Illegal offset type in form_type_checkboxes_value() (line 2229 of includes\form.inc).
      unset($existing_states[$sid]);
    }
  }
  foreach ($states as $state) {
    $sid = isset($state['sid']) ? $state['sid'] : NULL;
    unset($state['sid']);
    // Call the t() function to have the label added to the translation tables.
    t($state['label']);
    if (!empty($sid) && array_key_exists($sid, $existing_states)) {
      // Update
      db_update($table_name)
        ->fields($state)
        ->condition('sid', $sid, '=')
        ->execute();
    }
    else {
      // Insert, sid will be created.
      db_insert($table_name)
        ->fields($state)
        ->execute();
    }
  }
  drupal_static_reset('availability_calendar_get_states');
}

/**
 * Creates a new calendar an returns its id (the "cid").
 *
 * @return int
 *   The cid of the newly created calendar.
 */
function availability_calendar_create_calendar() {
  $now = time();
  $cid = db_insert('availability_calendar_calendar')
    ->fields(array('created' => $now, 'changed' => $now))
    ->execute();
  return (int) $cid;
}

/**
 * Updates the changed field of a calendar.
 *
 * @param int $cid
 *   The calendar id.
 */
function availability_calendar_update_calendar($cid) {
  $now = time();
  $cid = db_update('availability_calendar_calendar')
    ->fields(array('changed' => $now))
    ->condition('cid', $cid, '=')
    ->execute();
  return (int) $cid;
}

/**
 * Gets a calendar.
 *
 * @param int $cid
 *   The id of the calendar to get.
 * @return NULL|array
 *   Calendar record.
 */
function availability_calendar_get_calendar($cid) {
  $calendar = db_select('availability_calendar_calendar')
    ->fields('availability_calendar_calendar')
    ->condition('cid', $cid, '=')
    ->execute()
    ->fetchAssoc();
  return $calendar;
}

/**
 * Returns the availability for the given calendar and date range.
 *
 * The from and to dates are inclusive.
 *
 * @param int $cid
 *   cid may be 0 for not yet existing calendars.
 * @param DateTime $from
 * @param DateTime $to
 * @param int|NULL $default_state
 *   If $default_state is NULL only the stored availability is returned,
 *   otherwise the returned array is completed with this default state for
 *   missing dates.
 * @return array
 *   Array with availability within the given date range indexed by date.
 */
function availability_calendar_get_availability($cid, $from, $to, $default_state = NULL) {
  // Get the states from the database.
  $availability = array();
  if (!empty($cid)) {
    $availability = db_select('availability_calendar_availability')
      ->fields('availability_calendar_availability', array('date', 'sid'))
      ->condition('cid', $cid)
      ->condition('date', array($from->format(AC_ISODATE), $to->format(AC_ISODATE)), 'BETWEEN')
      ->execute()
      ->fetchAllKeyed();
  }
  if (!empty($default_state)) {
    for ($date = clone $from; $date <= $to; $date->modify('+1 day')) {
      $day = $date->format(AC_ISODATE);
      if (!array_key_exists($day, $availability)) {
        $availability[$day] = $default_state;
      }
    }
  }
  return $availability;
}

/**
 * Sets the given date range to the given state for the given calendar.
 *
 * Note that:
 * - $from and $to are both inclusive.
 * - $from and $to must be ordered.
 *
 * @param int $cid
 *   Th calendar id.
 * @param int $sid
 *   The state id
 * @param DateTime $from
 * @param DateTime $to
 * @param boolean $update_changed
 *   Whether to update the changed timestamp field of the calendar.
 *   Normally this should be set to true, but when called from
 *   availability_calendar_update_multiple_availability() it is set to false to
 *   prevent a sequence of similar writes to the field table.
 */
function availability_calendar_update_availability($cid, $sid, $from, $to, $update_changed = TRUE) {
  if ($update_changed) {
    availability_calendar_update_calendar($cid);
  }

  // Update the already existing dates.
  $count = db_update('availability_calendar_availability')
    ->fields(array('sid' => $sid))
    ->condition('cid', $cid, '=')
    ->condition('date', array($from->format(AC_ISODATE), $to->format(AC_ISODATE)), 'BETWEEN')
    ->execute();

  // Insert the non-existing dates.
  //PHP5.3: $days = $from->diff($to)->days + 1;
  $timestamp_from = (int) $from->format('U');
  $timestamp_to = (int) $to->format('U');
  $days = (int) round(($timestamp_to - $timestamp_from)/(60*60*24)) + 1;
  if ($count != $days) {
    // Get existing dates to know which ones to insert.
    $existing_availability = availability_calendar_get_availability($cid, $from, $to);

    $values = array('cid' => $cid, 'date' => NULL, 'sid' => $sid);
    $insert = db_insert('availability_calendar_availability')
      ->fields(array_keys($values));
    for ($day = clone $from; $day <= $to; $day->modify('+1 day')) {
      $values['date'] = $day->format(AC_ISODATE);
      if (!array_key_exists($values['date'], $existing_availability)) {
        $insert->values($values);
      }
    }
    $insert->execute();
  }
}

/**
 * Updates/inserts the states for the given ranges.
 *
 * @param NULL|int $cid
 *   The calendar id. If NULL, a new calendar will be created.
 * @param array $changes
 *   An array of changes each entry is an array with keys state, from and to.
 * @return int
 *   The cid (of the existing or newly created calendar).
 */
function availability_calendar_update_multiple_availability($cid, $changes) {
  if (empty($cid)) {
    $cid = availability_calendar_create_calendar();
  }
  else {
    availability_calendar_update_calendar($cid);
  }
  foreach ($changes as $change) {
    availability_calendar_update_availability($cid, $change['state'], $change['from'], $change['to'], FALSE);
  }
  return $cid;
}

/**
 * Adds a where clauses to the given query to filter on availability.
 *
 * Notes:
 * - The query will 'fail' when no calendar has been created yet and the default
 *   state is an "available" state. However I'm not sure whether this is a bug
 *   or correct behavior.
 * - If a field has multiple values, an entity may be returned multiple times.
 *   However this function cannot filter on duplicates as it does not know
 *   whether just calendars should be returned, or fields or entities, and
 *   whether duplicates are expected or not.
 *
 * @param SelectQuery|views_plugin_query $query
 *   The query to add the clause to. This may be a default Drupal or a Views
 *   specific query object.
 * @param string $field_table
 *   The name of the (field data) table to join on.
 * @param string $cid_field
 *   The name of the field in the table (to join on) that contains the cid.
 * @param DateTime $from
 *   The date to start searching for availability.
 * @param int|DateTime $to_or_duration
 *   Either the departure day (DateTime) or the duration of the stay (int).
 * @param int $default_state
 *   The sid of the state to use for dates without availability assigned.
 */
function availability_calendar_query_available($query, $field_table, $cid_field, $from, $to_or_duration, $default_state) {
  // Process parameters
  if (!$from instanceof DateTime) {
    return;
  }
  if ($to_or_duration instanceof DateTime) {
    $to = $to_or_duration;
    //PHP5.3: $duration = $arrival->diff($to)->days + 1;
    $timestamp_from = (int) $from->format('U');
    $timestamp_to = (int) $to->format('U');
    $diff = (int) round(($timestamp_to - $timestamp_from)/(60*60*24));
    $duration = $diff + 1;
  }
  else {
    $duration = (int) $to_or_duration;
    $diff = $duration -1;
    $to = clone $from;
    $to->modify("+$diff days");
  }

  // Check parameters.
  if ($duration <= 0) {
    return;
  }

  // Determine whether default availability state is to be treated as available.
  $states = availability_calendar_get_states();
  if (isset($states[$default_state])) {
    $default_state = $states[$default_state];
    $default_is_available = $default_state['is_available'] == 1;
  }
  else {
    // I have to make a choice :( (I could try to find out if there is only 1
    // calendar field or if the default is always the same or if the defaults
    // are always to be treated as either available or not available.)
    $default_is_available = FALSE;
  }
  $sids = array_keys(availability_calendar_get_states(!$default_is_available));

  $subquery = db_select('availability_calendar_availability', 'availability_calendar_availability')
    ->where("availability_calendar_availability.cid = $field_table.$cid_field")
    ->condition('availability_calendar_availability.date', array($from->format(AC_ISODATE), $to->format(AC_ISODATE)), 'BETWEEN')
    ->condition('availability_calendar_availability.sid', $sids, 'IN');
  if ($default_is_available) {
    // Default status = available: so no single day may be marked as non
    // available. Check by doing a check on the existence of a non available day
    // in the given period.
    $subquery->addExpression('1');
    if (is_a($query, 'views_plugin_query')) {
      $query->add_where(0, '', $subquery, 'NOT EXISTS');
    }
    else {
      $query->notexists($subquery);
    }
  }
  else {
    // Default status = not available: so all days must be marked as available.
    // Check by doing a count on the available days in the given period which
    // should equal the total number of days.
    $subquery->addExpression('count(*)');
    if (is_a($query, 'views_plugin_query')) {
      $query->add_where(0, $duration, $subquery, 'IN');
    }
    else {
      $query->condition($duration, $subquery, 'IN');
    }
  }
}

/**
 * Checks whether a calendar is available for the given period.
 *
 * @param int $cid
 * @param DateTime $from
 * @param int|DateTime $to_or_duration
 * @param int $default_state
 *   The sid of the state to use for dates without availability assigned.
 *
 * @return bool|null
 *   true or false to indicate whether the calendar is available in the given
 *   period, null if the period is not valid (negative duration).
 */
function availability_calendar_is_available($cid, $from, $to_or_duration, $default_state) {
  if ($to_or_duration instanceof DateTime) {
    $to = $to_or_duration;
    //PHP5.3: $duration = $arrival->diff($to)->days + 1;
    $timestamp_from = (int) $from->format('U');
    $timestamp_to = (int) $to->format('U');
    $diff = (int) round(($timestamp_to - $timestamp_from)/(60*60*24));
    $duration = $diff + 1;
  }
  else {
    $duration = (int) $to_or_duration;
    $diff = $duration - 1;
    $to = clone $from;
    $to->modify("+$diff days");
  }

  // Check parameters.
  if ($duration <= 0) {
    return NULL;
  }

  // Determine whether default availability state is to be treated as available.
  $states = availability_calendar_get_states();
  if (isset($states[$default_state])) {
    $default_state = $states[$default_state];
    $default_is_available = $default_state['is_available'] == 1;
  }
  else {
    // I have to make a choice :( (I could try to find out if there is only 1
    // calendar field or if the default is always the same or if the defaults
    // are always to be treated as either available or not available.)
    $default_is_available = FALSE;
  }

  // If the default status = available then no single day may be marked as non
  // available. Check by counting the non available days in the given period.
  // If the default status = non available then all days must be marked as
  // available. Check by counting the available days in the given period.
  //
  // Get the sids with opposite "treat as available".
  $sids = array_keys(availability_calendar_get_states(!$default_is_available));

  // Create and execute the count query and fetch the (scalar) result.
  $count = db_select('availability_calendar_availability')
    ->condition('cid', $cid)
    ->condition('date', array($from->format(AC_ISODATE), $to->format(AC_ISODATE)), 'BETWEEN')
    ->condition('sid', $sids, 'IN')
    ->countQuery()
    ->execute()
    ->fetchField();

  // Check the count (as explained above).
  return $default_is_available ? $count == 0 : $count == $duration;
}
