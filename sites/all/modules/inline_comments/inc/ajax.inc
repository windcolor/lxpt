<?php

/*
 ** @file
 *  ajax callback functions
 */

//$GLOBALS['uri'];
function inline_comments_add_comments() {

  global $user;
  //clean data and add comment to db
  $uid = check_plain($_POST['uid']);
  $nid = check_plain($_POST['nid']);
  $node = node_load($nid);
  $text = check_markup($_POST['comment_text']);
  $action = check_plain($_POST['action']);
  $action = preg_replace('/#.*/', '', $action);
  $action = preg_replace('/^\//', '', $action);
  $action = drupal_get_normal_path($action);
  $comment = (object) array(
      'nid' => $node->nid,
      'uid' => $user->uid,
      'cid' => 0,
      'pid' => 0,  // flat comments  in this module  no parent
      'mail' => '',
      'subject' => '',  // no subject
      'is_anonymous' => FALSE,
      'status' => COMMENT_PUBLISHED,
      'language' => LANGUAGE_NONE,
      'comment_body' => array(
        LANGUAGE_NONE => array(
          0 => array (
            'value' => $text,
            'format' => 'filtered_html'
          )
        )
      ),
    );
  //comment_submit($comment);
  comment_save($comment);
  $cids = comment_get_thread($node,  variable_get('comment_default_mode_' . $node->type, 1),  variable_get('comment_default_per_page_' . $node->type, 10));
  $comments = comment_load_multiple($cids);
  comment_prepare_thread($comments);
  //$comments = ctools_comment_render($node, $conf);
  drupal_add_css(drupal_get_path('module', 'comment') . '/comment.css');
  $build = comment_view_multiple($comments, $node);
  $build['pager']['#theme'] = 'pager';
  drupal_json_output(array( 'data' =>  drupal_render($build)));
}

function inline_comments_get_comments() {
  //clean data

  $nid = check_plain(arg(3));
  $node = node_load($nid);
  $cids = comment_get_thread($node,  variable_get('comment_default_mode_' . $node->type, 1),  variable_get('comment_default_per_page_' . $node->type, 10));
  $comments = comment_load_multiple($cids);
  comment_prepare_thread($comments);
  //$comments = ctools_comment_render($node, $conf);
  drupal_add_css(drupal_get_path('module', 'comment') . '/comment.css');
  $build = comment_view_multiple($comments, $node);
  $build['pager']['#theme'] = 'pager';
  drupal_json_output(array( 'data' =>  drupal_render($build)));

}


function inline_comments_get_comment_form() {
  //clean data
  if (module_exists('comment')) {
    global $user;
    $form = array();
    $nid = check_plain(arg(3));
    $pid = check_plain(arg(4));
    $node = node_load($nid);
    node_object_prepare($node);
    $output = drupal_get_form("comment_form" , $node);
    $output = drupal_render($output);
    drupal_json_output(array( 'data' => $output));
  }
  else {
    return;
  }
}

