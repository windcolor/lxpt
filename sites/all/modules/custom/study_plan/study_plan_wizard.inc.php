<?php

/**
 * @file
 * Extensible study plan wizard form .
 */



/**
 * 
 * init form multi steps
 *
 * @ingroup form_example
 */
function _form_example_steps() {
  return array(
      1 => array(
        'form' => 'study_plan_create_1',
      ),
      2 => array(
        'form' => 'study_plan_create_2',
      ),
     ); 
}

/**
 * The primary formbuilder function for the wizard form. This is the form that
 * you should call with drupal_get_form() from your code, and it will include
 * the rest of the step forms defined. You are not required to change this
 * function, as this will handle all the step actions for you.
 *
 * This form has two defined submit handlers to process the different steps:
 *  - Previous: handles the way to get back one step in the wizard.
 *  - Next:     handles each step form submission,
 *
 * The third handler, the finish button handler, is the default form _submit
 * handler used to process the information.
 *
 * You are not required to change the next or previous handlers, but you must
 * change the form_example_wizard_sbumit handler to perform the operations you
 * need on the collected information.
 *
 * @ingroup form_example
 */
function study_plan_wizard($form, &$form_state) {


  // Initialize a description of the steps for the wizard.
  if (empty($form_state['step'])) {
    $form_state['step'] = 1;
    
    // This array contains the function to be called at each step to get the
    // relevant form elements. It will also store state information for each
    // step.
    $form_state['step_information'] = _form_example_steps();
	//debug($form_state['step_information']);
  }
  $step = &$form_state['step'];
  //debug($step);
  if($step == 1){ 
  drupal_set_title(t('第@step步： 创建学习计划', array('@step' => $step)));
  }else{
  drupal_set_title(t('第@step步： 配置学习计划', array('@step' => $step)));
  }
  // Call the function named in $form_state['step_information'] to get the
  // form elements to display for this step.
  $form = $form_state['step_information'][$step]['form']($form, $form_state);
	
  // Show the 'previous' button if appropriate. Note that #submit is set to
  // a special submit handler, and that we use #limit_validation_errors to
  // skip all complaints about validation when using the back button. The
  // values entered will be discarded, but they will not be validated, which
  // would be annoying in a "back" button.
  if ($step > 1) {
    $form['prev'] = array(
      '#type' => 'submit',
      '#value' => t('上一步'),
      '#name' => 'prev',
      '#submit' => array('study_plan_wizard_previous_submit'),
      '#limit_validation_errors' => array(),
    );
	
  }

  // Show the Next button only if there are more steps defined.
  if ($step < count($form_state['step_information'])) {
     ////debug($step);
    // The Next button should be included on every step
    $form['next'] = array(
      '#type' => 'submit',
      '#value' => t('下一步'),
      '#name' => 'next',
      '#submit' => array('study_plan_wizard_next_submit'),
	  
    );
	
  }
  else {
    // Just in case there are no more steps, we use the default submit handler
    // of the form wizard. Call this button Finish, Submit, or whatever you
    // want to show. When this button is clicked, the
    // form_example_wizard_submit handler will be called.
    $form['next'] = array(
      '#type' => 'submit',
      '#value' => t('提交'),
    );
	
  }
 

  // Include each validation function defined for the different steps.
  if (function_exists($form_state['step_information'][$step]['form'] . '_validate')) {
    $form['next']['#validate'] = array($form_state['step_information'][$step]['form'] . '_validate');
	
  }
 
  return $form;
  
}

/**
 * Submit handler for the "previous" button.
 * - Stores away $form_state['values']
 * - Decrements the step counter
 * - Replaces $form_state['values'] with the values from the previous state.
 * - Forces form rebuild.
 *
 *
 */
function study_plan_wizard_previous_submit($form, &$form_state) {
  $current_step = &$form_state['step'];
  //debug($current_step);
  $form_state['step_information'][$current_step]['stored_values'] = $form_state['values'];
  //debug($form_state['step_information'][$current_step]['stored_values']);
  if ($current_step > 1) {
    $current_step--;
	////debug($current_step);
    $form_state['values'] = $form_state['step_information'][$current_step]['stored_values'];
	//debug($form_state['values']);
  }
  $form_state['rebuild'] = TRUE;
}

/**
 * Submit handler for the 'next' button.
 * - Saves away $form_state['values']
 * - Increments the step count.
 * - Replace $form_state['values'] from the last time we were at this page
 *   or with array() if we haven't been here before.
 * - Force form rebuild.
 *
 * 
 */
function study_plan_wizard_next_submit($form, &$form_state) {
  $current_step = &$form_state['step'];
  // debug($current_step);
  $form_state['step_information'][$current_step]['stored_values'] = $form_state['values'];
// debug($form_state['step_information'][$current_step]['stored_values']);
  

  if ($current_step < count($form_state['step_information'])) {
    $current_step++;
	debug($current_step);
    if (!empty($form_state['step_information'][$current_step]['stored_values'])) {
      $form_state['values'] = $form_state['step_information'][$current_step]['stored_values'];
	 //debug($form_state['values']);
    }
    else {
      $form_state['values'] = array();
	 
    }
    $form_state['rebuild'] = TRUE;
	
   return;
  }
 
}

/*
 *
 *Create the elements of the first part form
 *
 */
function study_plan_create_1($form, &$form_state) {
  $form = array();
  $machine_name = 'course';
  $term_id = 0;
  
   $options_first = study_plan_get_tax_term_options($term_id,$machine_name);

    // If we have a value for the first dropdown from $form_state['values'] we use
    // this both as the default value for the first dropdown and also as a
    // parameter to pass to the function that retrieves the options for the
    // second dropdown.
    $value_dropdown_first =  key($options_first);
   
  $form['plan_name'] = array(
    '#type' => 'textfield',
    '#title' => t('名称'),
    '#default_value' => !empty($form_state['values']['plan_name']) ? $form_state['values']['plan_name'] : '',
	'#required' => TRUE,
  );
  $form['plan_brief'] = array(
    '#type' => 'textarea',
    '#title' => t('描述'),
    '#default_value' => !empty($form_state['values']['plan_brief']) ? $form_state['values']['plan_brief'] : '',
	'#required' => TRUE,
  );
  $form['plan_branch'] = array(
    '#type' => 'select',
    '#title' => t('年级'),
	'#options' => $options_first,
    '#default_value' => !empty($form_state['values']['plan_branch']) ? $form_state['values']['plan_branch'] : '',
	'#ajax' => array(
            // When 'event' occurs, Drupal will perform an ajax request in the
            // background. Usually the default value is sufficient (eg. change for
            // select elements), but valid values include any jQuery event,
            // most notably 'mousedown', 'blur', and 'submit'.
            'event' => 'change',
            'callback' => 'taxterms_ajax_callback',
            'wrapper' => 'relation_grade_select_list',
        ),
	//'#required' => TRUE,
  );
 
  $form['plan_branch_1'] = array(
    '#type' => 'select',
    '#title' => t('科目'),
	'#prefix' => '<div id="relation_grade_select_list">',
    '#suffix' => '</div>',
	'#options' => study_plan_get_tax_term_options($value_dropdown_first,$machine_name),
    '#default_value' => !empty($form_state['values']['plan_branch_1']) ? $form_state['values']['plan_branch_1'] : '',
	
  );
  $form['plan_access'] = array(
    '#type' => 'radios',
    '#title' => t('权限'),
	'#options' => array(
						'public' => t('公开'),
						'private' => t('私有'),
						),
    '#default_value' => 'public',
	'#required' => TRUE,
  );
    //debug( $form);
  return $form;
}



function taxterms_ajax_callback($form, $form_state){

return $form['plan_branch_1'];


//$form_state['rebuild'] = TRUE;




}

// call  Course category
function study_plan_get_tax_term_options($term_id,$machine_name){
    if($term_id==0){ 
	$options = array( '0' => t('请选择年级'));
	}else{
	
	$options = array( '0' => t('请选择科目'));
	}

    $vid = taxonomy_vocabulary_machine_name_load($machine_name)->vid;

    $options_source = taxonomy_get_tree($vid);

    foreach($options_source as $item ) {
	    if($item->parents[0]==$term_id){ 
        $key = $item->tid;
        $value = $item->name;
        $options[$key] = $value;
		 
		 }
    }

    return $options;
}
/*function study_plan_get_tax_term_options_1($term_id,$machine_name){
    
	//debug($term_id);

    $vid = taxonomy_vocabulary_machine_name_load($machine_name)->vid;

    $options_source = taxonomy_get_tree($vid);

    foreach($options_source as $item ) {
	
	    if($item->parents[0]==){ 
        $key = $item->tid;
        $value = $item->name;
        $options[$key] = $value;
		 }
    }
    
    return $options;
}*/

function study_plan_create_1_validate($form, &$form_state) {
  if ($form_state['values']['plan_branch'] == '0') {
    form_set_error('plan_branch', t('请选择年级'));
  }


  if ($form_state['values']['plan_branch_1'] == '0') {
    form_set_error('plan_branch_1', t('请选择科目'));
  }

}
/*
 *
 *Create the elements of the second part form
 *
 */
function study_plan_create_2($form, &$form_state) {

  $form = array();
  
  //init stage count
  if (empty($form_state['num_names'])) {
     $form_state['num_names'] = 1;
   }

   //Define  fieldset container for stages
  $form['stages'] = array(
  '#type' => 'contianer',
  '#tree' => TRUE,
  '#prefix' => '<div id="stages">',
  '#suffix' => '</div>',
 );
 // output sstages
    for ($i = 0; $i < $form_state['num_names']; $i++) {
	   //debug($i);
      $form['stages'][$i] = array(
        '#type' => 'fieldset',
        '#tree' => TRUE,
		'#title' => t('学习阶段@count', array('@count' => $i+1)),
		'#prefix' => '<div class="two-col">',
     '#suffix' => '</div>'
      );
       
      $form['stages'][$i]['stage_name'] = array(
    '#type' => 'textfield',
    '#title' => t('阶段名称'),
    '#description' => t('请输入阶段名称'),
    '#required' => TRUE,
    '#default_value' => !empty($form_state['values']['stages'][$i]['stage_name']) ? $form_state['values']['stages'][$i]['stage_name'] : '',

  );
  //debug($form_state['values']['stages'][$i]['stage_name']);
  $form['stages'][$i]['stage_time'] = array(
    '#type' => 'textfield',
    '#title' => t('计划时长'),
    '#description' => t('请输入计划时长'),
    '#required' => TRUE,
    '#default_value' => !empty($form_state['values']['stages'][$i]['stage_time']) ? $form_state['values']['stages'][$i]['stage_time'] : '',
	'#size' => 4,
    //'#default_value' => variable_get('my_field', 0),
    '#element_validate' => array('element_validate_number'),
  );
  if($i>0){
  $form['stages'][$i]['remove_stage']= array(
  '#type' => 'submit',
  '#value' => t('删除'),
 
  '#name'=>'stage_clear',
   '#submit' => array('custom_ajax_remove_stage'),
   '#ajax' => array(
    'callback' => 'custom_ajax_stage',
    'wrapper' => 'stages',
   ),
     
  );
  }
  /*$form['#attached']['js'] = array(
	drupal_get_path('module','study_plan').'/remove_button.js' => array(
	'type' => 'file',
	),
	);*/
  
   
 }

 //}
 //Multi value fieldset
  $form['add_stage']= array(
  '#type' => 'submit',
  '#value' => t('添加阶段'),
  '#description' => t('点击创建新的学习阶段'),
  //'#href' => '',
  '#submit'=>array('custom_ajax_add_stage'),
  '#ajax' => array(
    'callback' => 'custom_ajax_stage',
    'wrapper' => 'stages',
   ),
  
  );
  
 //Cover image for study plan
  $form['file'] = array(
    '#type' => 'managed_file',
    '#title' => t('封面'),
    '#description' => t('请上传封面图片,文件格式: jpg, jpeg, png, gif'),
	'#upload_location' => 'public://',
	'#upload_validators' => array(
	'file_validate_extensions' => array('gif png jpg jpeg'),
	'file_validate_size' => array('MAX_FILE_SIZE' * 200 * 100),
	'#size' => 40,
	'#name' => 'cover',
	),
	
	
  );
  

//debug($form);
  
  return $form;
}


function custom_ajax_stage($form, $form_state) {
 
  return $form['stages'];
}

function custom_ajax_add_stage($form, &$form_state) {
 $form_state['num_names']++;
 ////debug( $form_state['num_names']++);
   $form_state['rebuild'] = TRUE;
  
}

function custom_ajax_remove_stage($form, &$form_state) {

     $form_state['num_names']--;
	 ////debug( $form_state['num_names']--);

   $form_state['rebuild'] = TRUE;
 
 
   
}


/**
 * Custom validation form for the 'location info' page of the wizard.
 *
 * This is the validation function for the second step of the wizard.
 * The city cannot be empty or be "San Francisco".
 *
 * @ingroup form_example
 */
function study_plan_create_2_validate($form, &$form_state) {
         
  if (!$form_state['values']['file']) { 
   //form_set_error('file', t('请上传封面'));
   form_set_error('file', t('请上传封面图片'));
  }
}  

/**
 * Returns form elements for the 'other info' page of the wizard. This is the
 * thid and last step of the example wizard.
 *
 * @ingroup form_example
 
function form_example_wizard_other_info($form, &$form_state) {
  $form = array();
  $form['aunts_name'] = array(
    '#type' => 'textfield',
    '#title' => t("Your first cousin's aunt's Social Security number"),
    '#default_value' => !empty($form_state['values']['aunts_name']) ? $form_state['values']['aunts_name'] : '',
  );
  return $form;
}

// And now comes the magic of the wizard, the function that should handle all the
// inputs from the user on each different step.
/**
 * Wizard form submit handler.
 * - Saves away $form_state['values']
 * - Process all the form values.
 *
 * This demonstration handler just do a drupal_set_message() with the information
 * collected on each different step of the wizard.
 *
 * @param $form
 * @param $form_state
 *
 * @ingroup form_example
 */
function study_plan_wizard_submit($form, &$form_state) {
  $current_step = &$form_state['step'];
  $form_state['step_information'][$current_step]['stored_values'] = $form_state['values'];
  
  $file =file_load($form_state['values']['file']);
   unset($form_state['values']['file']);
  $file->status = FILE_STATUS_PERMANENT;
  file_save($file);
  drupal_set_message(t('The form has been submitted and the image has been saved, filename: @filename.', array('@filename' => $file->filename)));
  
  // In this case we've completed the final page of the wizard, so process the
  // submitted information.
 
  drupal_set_message(t('创建成功'));
  
  foreach ($form_state['step_information'] as $index => $value) {
    // Remove FAPI fields included in the values (form_token, form_id and form_build_id
    // This is not required, you may access the values using $value['stored_values']
    // but I'm removing them to make a more clear representation of the collected
    // information as the complete array will be passed through drupal_set_message().
    unset($value['stored_values']['form_id']);
    unset($value['stored_values']['form_build_id']);
    unset($value['stored_values']['form_token']);

    // Now show all the values.
    drupal_set_message(t('Step @num collected the following values: <pre>@result</pre>', array('@num' => $index, '@result' => print_r($value['stored_values'], TRUE))));
  }
  //dsm($form);
}
