<?php
/**
 * @file
 * Views support for Availability Calendar.
 *
 * Availability Calendar supports the views module. As this is not a normal
 * field, we need to implement some views hooks and classes to get it to work.
 * Basically we provide:
 * - Fields:
 *   - Availability Calendar: because this is a field this works without custom
 *     code, whether the view displays content or fields.
 *   - Created date and changed date: though the current field formatters do not
 *     show this information, you can visualise them with a view.
 * - (Exposed) Filter criteria and arguments:
 *   - Enabled: only if there's a bundle that allows to override it on a per
 *     entity basis.
 *   - Created date.
 *   - Changed date.
 *   - Available.
 * - Sort criteria:
 *   - Created date.
 *   - Changed date.
 *
 * We also hide some things in the views UI:
 * - Argument/Filter:
 *   - Cid value.
 * - Sort:
 *   - Cid value.
 * - All revisioning options as basically a calendar is not revisioned.
 *   (@todo: look critically at this assumption.)
 *
 * @author Erwin Derksen (http://drupal.org/user/750928)
 */

/**
 * Implements hook_field_views_data_alter().
 *
 * The data structure contains too much non-descriptive information for me.
 * so we let views create the array for us first, then we alter it
 * (as opposed to implementing hook_field_views_data() itself).
 */
function availability_calendar_field_views_data_alter(&$data, $field, $module) {
  if ($field['type'] == 'availability_calendar') {
    // Get some info for easy use later on.
    $field_name = $field['field_name'];
    $field_table_name = key($field['storage']['details']['sql']['FIELD_LOAD_CURRENT']);
    $revision_table_name = key($field['storage']['details']['sql']['FIELD_LOAD_REVISION']);
    $entity_types = array_keys($field['bundles']);
    $field_views_info = $data[$field_table_name][$field_name];
    $field_title = $field_views_info['title'];
    // We should only join the availability_calendar_calendar table to a field
    // when the calendar is enabled: so we define this extra join condition.
    $join_extra = array(
      array(
        'table' => $field_table_name,
        'field' => "{$field_name}_enabled",
        'value' => 1,
        'numeric' => TRUE,
      ),
    );

    // Availability calendar field is not (really) revisioned: remove.
    unset($data[$revision_table_name]);

    // field_..._cid is not interesting in itself. Hide it as a field that could
    // be displayed, and use it only as a filter/argument on availability.
    $data[$field_table_name]["{$field_name}_cid"] = array(
      'group' => $field_views_info['group'],
      'title' => t('@field_label available', array('@field_label' => $field_title)),
      'title short' => t('@field_label available', array('@field_label' => $field_title)),
      'help' => t('Filters on availability during the defined period.') . ' ' . $field_views_info['help'],
      'argument' => array(
        'field' => "{$field_name}_cid",
        'field_name' => "{$field_name}_cid",
        'table' => $field_table_name,
        'handler' => 'views_handler_argument_date',
      ),
      'filter' => array(
        'field' => "{$field_name}_cid",
        'field_name' => "{$field_name}_cid",
        'table' => $field_table_name,
        'handler' => 'availability_calendar_handler_filter_availability',
        'default_state' => $field['settings']['default_state'],
        'allocation_type' => $field['settings']['allocation_type'],
      ),
    );

    // field_..._name:
    // - Change UI texts.
    $data[$field_table_name]["{$field_name}_name"]['title'] = t('@field_label name', array('@field_label' => $field_title));
    $data[$field_table_name]["{$field_name}_name"]['title short'] = t('@field_label name', array('@field_label' => $field_title));
    $data[$field_table_name]["{$field_name}_name"]['help'] = t('The name of the calendar', array('@field_label' => $field_title));

    // field_..._enabled:
    // - Change UI texts.
    $data[$field_table_name]["{$field_name}_enabled"]['title'] = t('@field_label enabled', array('@field_label' => $field_title));
    $data[$field_table_name]["{$field_name}_enabled"]['title short'] = t('@field_label enabled', array('@field_label' => $field_title));
    $data[$field_table_name]["{$field_name}_enabled"]['help'] = t('Whether the calendar has been enabled', array('@field_label' => $field_title));
    $data[$field_table_name]["{$field_name}_enabled"]['filter']['handler'] = 'views_handler_filter_boolean_operator';

    // Table availability_calendar_calendar:
    // - Define the intermediate join table and an extra join condition on
    //   enabled.
    // - As there may be multiple calendar fields, the array entry must be
    //   unique. So we cannot use the table name but must use an alias.
    //   Therefore the real table name must be mentioned in the join defintion.
    // - Add fields cid, created and changed. Cid will be the placeholder for
    //   filtering on availability (as cid itself is not very useful)
    $table_availability_calendar_calendar = array();
    foreach ($entity_types as $entity_type) {
      $table_availability_calendar_calendar['table']['join'][$entity_type] = array(
        'table' => 'availability_calendar_calendar',
        'left_table' => $field_table_name,
        'left_field' => "{$field_name}_cid",
        'field' => 'cid',
        'extra' => $join_extra,
      );
    }
    $table_availability_calendar_calendar['created'] = array(
      'group' => $field_views_info['group'],
      'title' => t('@field_label created date', array('@field_label' => $field_title)),
      'title short' => t('@field_label created', array('@field_label' => $field_title)),
      'help' => t('The date the calendar was created.') . ' ' . $field_views_info['help'],
      'field' => array(
        'field_name' => 'created',
        'table' => 'availability_calendar_calendar',
        'handler' => 'views_handler_field_date',
        'click sortable' => true,
        'element type' => 'span',
      ),
      'argument' => array(
        'field' => 'created',
        'field_name' => 'created',
        'table' => 'availability_calendar_calendar',
        'handler' => 'views_handler_argument_date',
      ),
      'filter' => array(
        'field' => 'created',
        'field_name' => 'created',
        'table' => 'availability_calendar_calendar',
        'handler' => 'views_handler_filter_date',
        'allow empty' => FALSE,
      ),
      'sort' => array(
        'field' => 'created',
        'field_name' => 'created',
        'table' => 'availability_calendar_calendar',
        'handler' => 'views_handler_sort_date',
      ),
    );
    $table_availability_calendar_calendar['changed'] = array(
      'group' => $field_views_info['group'],
      'title' => t('@field_label updated date', array('@field_label' => $field_title)),
      'title short' => t('@field_label updated', array('@field_label' => $field_title)),
      'help' => t('The date the calendar was last updated.') . ' ' . $field_views_info['help'],
      'field' => array(
        'field_name' => 'changed',
        'table' => 'availability_calendar_calendar',
        'handler' => 'views_handler_field_date',
        'click sortable' => true,
        'element type' => 'span',
      ),
      'argument' => array(
        'field' => 'changed',
        'field_name' => 'changed',
        'table' => 'availability_calendar_calendar',
        'handler' => 'views_handler_argument_date',
      ),
      'filter' => array(
        'field' => 'changed',
        'field_name' => 'changed',
        'table' => 'availability_calendar_calendar',
        'handler' => 'views_handler_filter_date',
        'allow empty' => FALSE,
      ),
      'sort' => array(
        'field' => 'changed',
        'field_name' => 'changed',
        'table' => 'availability_calendar_calendar',
        'handler' => 'views_handler_sort_date',
      ),
    );
    // Use a unique alias name.
    $data['availability_calendar_calendar_' . $field_table_name] = $table_availability_calendar_calendar;
  }
}
