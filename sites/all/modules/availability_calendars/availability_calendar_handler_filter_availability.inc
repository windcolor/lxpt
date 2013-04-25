<?php

/**
 * @class availability_calendar_handler_filter_availability
 * Views handler to filter on availability.
 *
 * This filter inherits from @see views_handler_filter as inheriting from
 * views_handler_filter_numeric or views_handler_filter_date did not turn out to
 * be a lot easier.
 *
 * This filter allows to filter on availability by accepting the following
 * values:
 * - 1 date (for availability at that given date)
 * - Begin and end date (end date inclusive)
 * - Arrival and departure date (departure date not inclusive)
 * - Start date and duration
 */
class availability_calendar_handler_filter_availability extends views_handler_filter {
  public static $instance;

  public function __construct() {
    self::$instance = $this;
    $this->always_multiple = TRUE;
    module_load_include('inc', 'availability_calendar', 'availability_calendar');
  }

  public function option_definition() {
    $options = parent::option_definition();

    $options['value'] = array(
      'contains' => array(
        'from' => array('default' => ''),
        'to' => array('default' => ''),
        'to1' => array('default' => ''),
        'duration' => array('default' => ''),
      ),
    );
    $options['operator'] = array('default' => 'from_duration');

    return $options;
  }

  public function operators() {
    $operators = array(
      'at' => array(
        'title' => t('At (date)'),
        'method' => 'op_at',
        'summary' => t('at %from'),
        'values' => array('from'),
      ),
      'from_to' => array(
        'title' => t('From begin up to and including end'),
        'method' => 'op_from_to',
        'summary' => t('From %from to %to'),
        'values' => array('from', 'to'),
      ),
      'from_to1' => array(
        'title' => t('From arrival to departure'),
        'method' => 'op_from_to1',
        'summary' => t('From %from to %to1'),
        'values' => array('from', 'to1'),
      ),
      'from_duration' => array(
        'title' => t('From begin during duration'),
        'method' => 'op_from_duration',
        'summary' => t('From %from during %duration days'),
        'values' => array('from', 'duration'),
      ),
    );

    return $operators;
  }

  /**
   * Provides a list of all the availability operators, optionally restricted
   * to only the given property of the operators.
   */
  public function operator_options($which = 'title') {
    $options = array();
    foreach ($this->operators() as $id => $operator) {
      $options[$id] = $operator[$which];
    }
    return $options;
  }

  public function operators_by_value($value) {
    $options = array();
    foreach ($this->operators() as $id => $operator) {
      if (in_array($value, $operator['values'])) {
        $options[] = $id;
      }
    }
    return $options;
  }

  /**
   * Add validation and date popup(s) to the value form.
   */
  public function value_form(&$form, &$form_state) {
    $form['value']['#tree'] = TRUE;

    if (empty($form_state['exposed'])) {
      // We're in Views edit mode self. Add validator here. When we're in an
      // exposed form, validation will go via exposed_validate().
      $form['value']['#element_validate'][] = 'availability_calendar_handler_filter_availability_validate_value';
    }

    // Determine values to add and their dependencies.
    $dependency_source = NULL;
    if (!empty($form_state['exposed']) && (empty($this->options['expose']['use_operator']) || empty($this->options['expose']['operator_id']))) {
      // Exposed form with operator not exposed: only add values for the
      // set operator.
      $operators = $this->operators();
      $values = $operators[$this->operator]['values'];
    }
    else {
      // Views UI
      $values = array('from', 'to', 'to1', 'duration');
      if (!empty($form['operator'])) {
        $dependency_source = $form['operator']['#type'] === 'radios' ? 'radio:options[operator]' : 'edit-options-operator';
      }
    }

    // Add value fields.
    if (in_array('from', $values)) {
      $form['value']['from'] = array(
        '#type' => 'textfield',
        '#title' => $this->operator === 'at' ? t('At') : ($this->operator === 'from_to1' ? t('Arrival date') : t('Start date')),
        '#size' => 12,
        '#default_value' => $this->value['from'],
      );
      if (module_exists('date_popup')) {
        $this->change_element_into_date_popup($form['value']['from']);
      }
      else {
        $date_example = availability_calendar_format_entry_date(new DateTime());
        $form['value']['from']['#description'] = t('E.g., @date', array('@date' => $date_example));
      }
      if ($dependency_source !== NULL) {
        $form['value']['from']['#dependency'] = array($dependency_source => $this->operators_by_value('from'));
      }
    }
    if (in_array('to', $values)) {
      $form['value']['to'] = array(
        '#type' => 'textfield',
        '#title' => t('End date'),
        '#size' => 12,
        '#default_value' => $this->value['to'],
      );
      if (module_exists('date_popup')) {
        $this->change_element_into_date_popup($form['value']['to']);
      }
      else {
        $date = new DateTime();
        $date->modify('+6 days');
        $date_example = availability_calendar_format_entry_date($date);
        $form['value']['to']['#description'] = t('E.g., @date', array('@date' => $date_example));
      }
      if ($dependency_source !== NULL) {
        $form['value']['to']['#dependency'] = array($dependency_source => $this->operators_by_value('to'));
      }
    }
    if (in_array('to1', $values)) {
      $form['value']['to1'] = array(
        '#type' => 'textfield',
        '#title' => t('Departure date'),
        '#size' => 12,
        '#default_value' => $this->value['to1'],
      );
      if (module_exists('date_popup')) {
        $this->change_element_into_date_popup($form['value']['to1']);
      }
      else {
        $date = new DateTime();
        $date->modify('+7 days');
        $date_example = availability_calendar_format_entry_date($date);
        $form['value']['to1']['#description'] = t('E.g., @date', array('@date' => $date_example));
      }
      if ($dependency_source !== NULL) {
        $form['value']['to1']['#dependency'] = array($dependency_source => $this->operators_by_value('to1'));
      }
    }
    if (in_array('duration', $values)) {
      $options = array(0 => t('- Select duration -'));
      for($i = 1; $i <= 28; $i++) {
        if ($i % 7 === 0) {
          $options[$i] = format_plural($i / 7, '1 week', '@count weeks');
        }
        else if ($i <= 20) {
          // Keep localize.drupal.org satisfied.
          if ($this->definition['allocation_type'] === ALLOCATION_TYPE_FULLDAY) {
            $options[$i] = format_plural($i, '1 day', '@count days');
          }
          else {
            $options[$i] = format_plural($i, '1 night', '@count nights');
          }
        }
      }
      $form['value']['duration'] = array(
        '#type' => 'select',
        '#title' => t('Duration'),
    		'#options' => $options,
      	'#default_value' => $this->value['duration'],
      );
      if ($dependency_source !== NULL) {
        $form['value']['duration']['#dependency'] = array($dependency_source => $this->operators_by_value('duration'));
      }
    }
  }

  /**
   * Changes a (text) form element into a dete popup element.
   *
   * @param array $element
   */
  protected function change_element_into_date_popup(&$element) {
    $element['#type'] = 'date_popup';
    $element['#date_label_position'] = '';
    $element['#date_type'] = 'DATE_ISO';
    $element['#date_format'] = variable_get('date_format_availability_calendar_date_entry', '');
    $element['#date_year_range'] = '-0:+2';
  }

  /**
   * Validates our part of the exposed form.
   *
   * Overrides @see views_handler::exposed_validate().
   */
  public function exposed_validate(&$form, &$form_state) {
    if (empty($this->options['exposed'])) {
      return;
    }
    $this->validate_value($form[$this->options['expose']['identifier']], $form_state);
  }

  /**
   * Validate that the values convert to something usable.
   */
  public function validate_value(&$element, $form_state) {
    if (empty($form_state['exposed'])) {
      // In Views UI, the value is required if the filter is not exposed,
      // otherwise we don't validate at all (so people can place "help texts" in
      // the inputs.)

      if ($form_state['values']['options']['expose_button']['checkbox']['checkbox']) {
        return;
      }
      $required = FALSE;
      $operator = $form_state['values']['options']['operator'];
    }
    else {
      // In exposed form, values are required if "Required" was checked.
      $required = (bool) $this->options['expose']['required'];
      $operator = empty($this->options['expose']['use_operator']) ? $this->operator : $form_state['values'][$this->options['expose']['operator_id']];
    }
    $operators = $this->operators();
    $values = empty($operator) ? array('from', 'to', 'to1', 'duration') : $operators[$operator]['values'];

    // Set time to midnight as other dates are also set to that time.
    $today = new DateTime();
    $today->setTime(0, 0, 0);

    $value = drupal_array_get_nested_value($form_state['values'], $element['#parents']);
    $from_valid = FALSE;
    if (in_array('from', $values) && array_key_exists('from', $value)) {
      $from_valid = $this->validate_valid_time_1($element['from'], $value['from'], $required, $today, t('Only future availability can be searched.'));
    }
    if (in_array('to', $values) && array_key_exists('to', $value)) {
      $this->validate_valid_time_1($element['to'], $value['to'], $required || $from_valid instanceof DateTime, $from_valid, t('The end date should be on or after the start date.'));
    }
    if (in_array('to1', $values) && array_key_exists('to1', $value)) {
      $this->validate_valid_time_1($element['to1'], $value['to1'], $required || $from_valid instanceof DateTime, $from_valid, t('The departure date should be after the arrival date.'));
    }
    if (in_array('duration', $values) && array_key_exists('duration', $value)) {
      $this->validate_valid_duration($element['duration'], $value['duration'], $required || $from_valid instanceof DateTime);
    }
  }

  /**
   * @param array $element
   * @param array|DateTime $value
   * @param bool $required
   * @param DateTime|null $minimum
   * @param string $minimum_error_message
   * @return DateTime|false
   */
  protected function validate_valid_time_1(&$element, $value, $required, $minimum, $minimum_error_message) {
    $valid = TRUE;
    // If date popup is enabled, the value will be an array (with a date and
    // time component).
    if (is_array($value)) {
      $value = $value['date'];
    }
    if (empty($value) || $value === $element['#default_value']) {
      if ($required) {
        form_error($element, t('Field %field is required.', array('%field' => $element['#title'])));
        $valid = FALSE;
      }
    }
    else if (($value = availability_calendar_parse_entry_date($value)) === FALSE) {
      form_error($element, t('Invalid date format.'));
      $valid = FALSE;
    }
    else if ($minimum instanceof DateTime && $value < $minimum) {
      form_error($element, $minimum_error_message);
      $valid = FALSE;
    }
    return $valid ? $value : $valid;
  }

  protected function validate_valid_duration(&$element, $value, $required) {
    $valid = TRUE;
    if (empty($value)) {
      if ($required) {
        form_error($element, t('Field %field is required.', array('%field' => $element['#title'])));
        $valid = FALSE;
      }
    }
    else if ((!is_int($value) && !ctype_digit($value)) || $value <= 0) {
      form_error($element, t('Duration must be a positive number of days.'));
      $valid = FALSE;
    }
    return $valid;
  }

  /**
  * Check to see if input from the exposed filters should change
  * the behavior of this filter.
  */
  public function accept_exposed_input($input) {
    if (empty($this->options['exposed'])) {
      return TRUE;
    }

    if (!empty($this->options['expose']['use_operator']) && !empty($this->options['expose']['operator_id']) && isset($input[$this->options['expose']['operator_id']])) {
      // Fetch operator from form (instead of from $this object)
      $this->operator = $input[$this->options['expose']['operator_id']];
    }

    if (!empty($this->options['expose']['identifier'])) {
      // Fetch value from form (instead of from $this object)
      $this->value = $input[$this->options['expose']['identifier']];

      // Check if the values are filled in, if not, we don't want to change the
      // query. A value is filled in if it is not empty and does not equal an
      // invalid default value. Validation will already have failed if the value
      // does not equal the default value but is invalid. So we just check if
      // the values are valid.
      $operators = $this->operators();
      $values = $operators[$this->operator]['values'];
      foreach ($values as $value_name) {
        if (empty($this->value[$value_name])) {
          return FALSE;
        }
        else if ($value_name === 'duration') {
          if ((!is_int($this->value[$value_name]) && !ctype_digit($this->value[$value_name])) || $this->value[$value_name] <= 0) {
            return FALSE;
          }
        }
        else {
          if (availability_calendar_parse_entry_date($this->value[$value_name]) === FALSE) {
            return FALSE;
          }
        }
      }
    }

    return TRUE;
  }

  public function query() {
    $this->ensure_my_table();
    $operators = $this->operators();
    if (isset($operators[$this->operator]['method'])) {
      $this->{$operators[$this->operator]['method']}();
    }
  }

  protected function op_at() {
    $this->value['duration'] = 1;
    $this->op_from_duration();
  }

  protected function op_from_to() {
    module_load_include('inc', 'availability_calendar', 'availability_calendar.widget');
    availability_calendar_query_available($this->query,
      $this->table_alias,
      $this->real_field,
      new DateTime($this->value['from']),
      new DateTime($this->value['to']),
      $this->definition['default_state']);
  }

  protected function op_from_to1() {
    $from = new DateTime($this->value['from']);
    $to = new DateTime($this->value['to1']);
    if ($from instanceof DateTime && $to instanceof DateTime) {
      // Departure date (to1) is not inclusive. So we modify it by 1 day.
      // But we do accept the same dates for arrival (from) and departure (to1).
      // In that case we leave the to date as is (equal to the from date).
      if ($to > $from) {
        $to->modify('-1 day');
      }
      module_load_include('inc', 'availability_calendar', 'availability_calendar.widget');
      availability_calendar_query_available($this->query,
        $this->table_alias,
        $this->real_field,
        $from,
        $to,
        $this->definition['default_state']);
    }
  }

  protected function op_from_duration() {
    module_load_include('inc', 'availability_calendar', 'availability_calendar.widget');
    availability_calendar_query_available($this->query,
      $this->table_alias,
      $this->real_field,
      new DateTime($this->value['from']),
      (int) $this->value['duration'],
      $this->definition['default_state']);
  }

  public function admin_summary() {
    $output = '';
    if (!empty($this->options['exposed'])) {
      $output = t('exposed');
    }
    else {
      $operators = $this->operators();
      if (isset($operators[$this->operator]['summary'])) {
        $arguments = array();
        foreach($this->value as $key => $value) {
          $arguments["@$key"] = $value;
        }
        $output = format_string($operators[$this->operator]['summary'], $arguments);
      }
    }
    return $output;
  }
}

/**
 * Form element validator for the date field(s): forward to the
 * availability_calendar_handler_filter_availability::validate_value()
 * method inside the class.
 */
function availability_calendar_handler_filter_availability_validate_value(&$element, &$form_state, $form) {
  availability_calendar_handler_filter_availability::$instance->validate_value($element, $form_state);
}

/**
 * Called by hook_alter_js().
 *
 * This function changes the date popups added by the class above.
 */
function availability_calendar_handler_filter_availability_js_alter(&$javascript) {
  static $adapt = FALSE;
  if ($javascript === TRUE) {
    $adapt = TRUE;
    return;
  }
  if ($adapt && isset($javascript['settings']['data'])) {
    foreach ($javascript['settings']['data'] as &$setting) {
      if (is_array($setting) && isset($setting['datePopup'])) {
        foreach ($setting['datePopup'] as &$date_popup_settings) {
          $date_popup_settings['settings']['minDate'] = 0;
        }
      }
    }
  }
}
