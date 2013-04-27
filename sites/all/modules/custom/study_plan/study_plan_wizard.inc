<?php

/**
 * @file
 * Extensible wizard form example.
 */

/**
 * Extensible wizard form example.
 *
 * This is an example of a multistep form using a wizard style. It will include
 * the 'Previous' and 'Next' buttons when required, and a 'Finish' button at the
 * last stage of the form submission.
 *
 * This example is an extensible skeleton that can include (even
 * programatically) more steps. The demonstration form includes three steps,
 * each step having its own validation functions.
 *
 * How to extend this example:
 * - Steps are defined in the _form_example_steps() function. Include or alter
 *   the steps as you require.
 * - For each step, implement the corresponding 'form' function (see
 *   'form_example_wizard_personal_info' for the first step in this example.)
 *   Each step is a regular form, and the wizard collects all the values of the
 *   included forms.
 * - Optionally, you may include custom validation functions using the regular
 *   validation hook (formname_validate). The wizard uses these validation
 *   functions for each step.
 * - The most important customization step is to change the submit handler and
 *   do whatever you want with the collected information. In this case, the
 *   example just shows the collected values in the various steps.
 * @ingroup form_example
 */

/**
 * Returns the list of steps and their associated forms. This has been separated
 * to clarify and easy the understanding of this example. You should edit this
 * function to include the steps your wizard/multistep form requires.
 *
 * @return $array
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
	//debug($form);
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
	$dd='上一步';
	//debug($dd);
  }

  // Show the Next button only if there are more steps defined.
  if ($step < count($form_state['step_information'])) {
     //debug($step);
    // The Next button should be included on every step
    $form['next'] = array(
      '#type' => 'submit',
      '#value' => t('下一步'),
      '#name' => 'next',
      '#submit' => array('study_plan_wizard_next_submit'),
	  '#limit_validation_errors' => array(),
    );
	$ee='下一步';
	//debug($ee);
  }
  else {
    // Just in case there are no more steps, we use the default submit handler
    // of the form wizard. Call this button Finish, Submit, or whatever you
    // want to show. When this button is clicked, the
    // form_example_wizard_submit handler will be called.
    $form['finish'] = array(
      '#type' => 'submit',
      '#value' => t('提交'),
    );
	$ff='提交';
	//debug($ff);
  }

  // Include each validation function defined for the different steps.
  if (function_exists($form_state['step_information'][$step]['form'] . '_validate')) {
    $form['next']['#validate'] = array($form_state['step_information'][$step]['form'] . '_validate');
	//debug($form['next']['#validate']);
  }
  //debug ($form);
  return $form;
  
}

/**
 * Submit handler for the "previous" button.
 * - Stores away $form_state['values']
 * - Decrements the step counter
 * - Replaces $form_state['values'] with the values from the previous state.
 * - Forces form rebuild.
 *
 * You are not required to change this function.
 *
 * @ingroup form_example
 */
function study_plan_wizard_previous_submit($form, &$form_state) {
  $current_step = &$form_state['step'];
  //debug($current_step);
  $form_state['step_information'][$current_step]['stored_values'] = $form_state['values'];
  //debug($form_state['step_information'][$current_step]['stored_values']);
  if ($current_step > 1) {
    $current_step--;
	//debug($current_step);
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
 * You are not required to change this function.
 *
 * @param $form
 * @param $form_state
 *
 * @ingroup form_example
 */
function study_plan_wizard_next_submit($form, &$form_state) {
  $current_step = &$form_state['step'];
   //debug($current_step);
  $form_state['step_information'][$current_step]['stored_values'] = $form_state['values'];///////////////////////////////
  //debug($form_state['step_information'][$current_step]['stored_values']);
  

  if ($current_step < count($form_state['step_information'])) {
    $current_step++;
	//debug($current_step);
    if (!empty($form_state['step_information'][$current_step]['stored_values'])) {
      $form_state['values'] = $form_state['step_information'][$current_step]['stored_values'];
	 //debug($form_state['values']);
    }
    else {
      $form_state['values'] = array();
	  //debug($form_state['values']);
    }
    $form_state['rebuild'] = TRUE;
	// Force rebuild with next step.
	 //debug( $form);
   return;
  }
 
}

/**
 * The previous code was a 'skeleton' of a multistep wizard form. You are not
 * required to change a line on the previous code (apart from defining your own
 * steps in the _form_example_steps() function.
 *
 * All the code included from here is the content of the wizard, the steps of
 * the form.
 *
 * First, let's show the defined steps for the wizard example.
 * @ingroup form_example
 */

/**
 * Returns form elements for the 'personal info' page of the wizard. This is the
 * first step of the wizard, asking for two textfields: first name and last
 * name.
 *
 * @ingroup form_example
 */
function study_plan_create_1($form, &$form_state) {
  $form = array();
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
    '#title' => t('分类'),
	'#options' => study_plan_get_tax_term_options('course'),
    '#default_value' => !empty($form_state['values']['plan_branch']) ? $form_state['values']['plan_branch'] : '',
	'#required' => TRUE,
  );
  $form['plan_access'] = array(
    '#type' => 'radios',
    '#title' => t('权限'),
	'#options' => array(
						'public' => t('公开'),
						'private' => t('私有'),
						),
    '#default_value' => !empty($form_state['values']['plan_access']) ? $form_state['values']['plan_access'] : '',
	'#required' => TRUE,
  );
    //debug( $form);
  return $form;
}

// call  Course category



function study_plan_get_tax_term_options($machine_name){
    $options = array( '0' => t('请选择计划分类'));

    $vid = taxonomy_vocabulary_machine_name_load($machine_name)->vid;

    $options_source = taxonomy_get_tree($vid);

    foreach($options_source as $item ) {
        $key = $item->tid;
        $value = $item->name;
        $options[$key] = $value;
    }

    return $options;
}

function study_plan_create_1_validate($form, &$form_state) {
  if ($form_state['values']['plan_branch'] == '0') {
    form_set_error('city', t('请选择计划分类'));
  }

}
/**
 * Returns form elements for the 'location info' page of the wizard. This is the
 * second step of the wizard. This step asks for a textfield value: a City. This
 * step also includes a validation declared later.
 *
 * @ingroup form_example
 */
function study_plan_create_2($form, &$form_state) {

  //$form = array();
  //init stage count
  if (empty($form_state['num_names'])) {
     $form_state['num_names'] = 1;
   }
  /*$form_state['count']['stages'] =
    isset($form_state['count']['stages']) ? $form_state['count']['stages'] : 0;*/
    //debug( $form_state['count']['stages']);
   //Define fieldset container
  $form['stages'] = array(
  '#type' => 'contianer',
  '#tree' => TRUE,
  '#prefix' => '<div id="stages">',
  '#suffix' => '</div>',
 );
 //debug($form['stages']);
  /*test element
  $form['stages'][0] = array(
        '#type' => 'fieldset',
        '#tree' => TRUE,
		'#title' => t('学习阶段1'),
      );
	  
  $form['stages'][0]['stage_name'] = array(
    '#type' => 'textfield',
    '#title' => t('阶段名称'),
    '#description' => t('请输入阶段名称'),
    '#required' => TRUE,
    '#default_value' => !empty($form_state['stages'][0]['stage_name']) ? $form_state['stages'][0]['stage_name'] : '',

  );
  
  $form['stages'][0]['stage_time'] = array(
    '#type' => 'textfield',
    '#title' => t('计划时长'),
    '#description' => t('请输入计划时长'),
    '#required' => TRUE,
    '#default_value' => !empty($form_state['stages'][0]['stage_time']) ? $form_state['stages'][0]['stage_time'] : '',
	'#size' => 4,
    //'#default_value' => variable_get('my_field', 0),
    '#element_validate' => array('element_validate_number'),
  );*/
 
 // stage element
 //if ($form_state['count']['stages']) {
	 //$form_state['num_names'] = 1;
	 //$i=$form_state['num_names'];
    for ($i = 0; $i < $form_state['num_names']; $i++) {
	   debug($i);
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
    //'#default_value' => !empty($form_state['stages'][0]['stage_name']) ? $form_state['stages'][0]['stage_name'] : '',

  );
  
  $form['stages'][$i]['stage_time'] = array(
    '#type' => 'textfield',
    '#title' => t('计划时长'),
    '#description' => t('请输入计划时长'),
    '#required' => TRUE,
    //'#default_value' => !empty($form_state['stages'][0]['stage_time']) ? $form_state['stages'][0]['stage_time'] : '',
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
	//required' => TRUE,
  );
  
  
debug($form);
  //count stages
   //$form_state['count']['stages']++; 
   //count stages
   //debug($form_state['count']['stages']);
  //count stages
   //$form_state['count']['stages']++;
  return $form;
}


function custom_ajax_stage($form, $form_state) {
 
  return $form['stages'];
}

function custom_ajax_add_stage($form, &$form_state) {
 $form_state['num_names']++;
 //debug( $form_state['num_names']++);
   $form_state['rebuild'] = TRUE;
  
}

function custom_ajax_remove_stage($form, &$form_state) {

     $form_state['num_names']--;
	 //debug( $form_state['num_names']--);

   $form_state['rebuild'] = TRUE;
 
 
   
}
/*function custom_ajax_remove_stage($form, &$form_state) {
 if ($form_state['num_names'] > 1) {
	
	unset($form['stages'][$i]);
     $form_state['num_names']--;
   }
   $form_state['rebuild'] = TRUE;
 
 
  
  //$form_state['num_names']--;

  //$form_state['rebuild'] = TRUE;
}*/

/**
 * Custom validation form for the 'location info' page of the wizard.
 *
 * This is the validation function for the second step of the wizard.
 * The city cannot be empty or be "San Francisco".
 *
 * @ingroup form_example
 */
function study_plan_create_2_validate($form, &$form_state) {
  
  if ($form_state['values']['file']==NULL) { 
   form_set_error('file', t('请上传封面'));
  }
  /*$file = file_save_upload('file', array(
      'file_validate_is_image' => array(),
      'file_validate_extensions' => array('png gif jpg jpeg'),
	  'public://'. $file->filename, FILE_EXISTS_REPLACE
  ));
 
  
  if ($file) {
         
	 $form_state['values']['file'] = $file->fid;
   }
   else {
 
    form_set_error('file', t('上传封面出错'));
	}*/
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
    
  $file =file_load( $form_state['values']['file']);
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
  dsm($form);
}
