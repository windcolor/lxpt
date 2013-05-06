<?php 

function import_to_node_with_entity_api($posted_vlaues ){

global $user;
$store_values = array(
  'type' => 'study_plan',
  'uid' => $user->uid,
  'status' => 1,
  'comment' => 1,
  'promote' => 0,
  //'group_group'=> 1,
  'title' => $posted_vlaues['plan_name'],
  //'body' => $stored_vlaues[plan_brief],
  
 //'image_file' => $stored_vlaues[file],*/
  
  
  
);

// Create an Entity
$e = entity_create('node',$store_values);
//dsm($e);
// Specify the author

// Create a Entity Wrapper of that new Entity
$entity = entity_metadata_wrapper('node',$e);
//dsm($entity);
// Specify the title
$entity->field_branch =$posted_vlaues['plan_branch'];
$entity->field_textbook =$posted_vlaues['plan_textbook'];
// Save the node.

$entity->save();
// redirect to the new node main page after the entity is saved 
//debug($e);
//$entity_id = array_shift(entity_extract_ids('node',$e));//get entity id
global $base_url;
drupal_goto($base_url . '/node/' . $e->nid);






}