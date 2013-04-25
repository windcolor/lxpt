<?php

/*
 ** @file
 *  views handler class
 *  mostyly  just  renders the field for views display  and adds  js and css
 */

class inline_comments_handler_field_inline_comments extends views_handler_field {
  var $output;
  
  function query(){
    drupal_add_css(drupal_get_path('module', 'comments') . '/comment.css');
    drupal_add_css(drupal_get_path('module', 'inline_comments') . '/css/inline_comments_views.css');
    drupal_add_js(drupal_get_path('module', 'inline_comments') . '/js/loadComments.js');
    drupal_add_js(drupal_get_path('module', 'inline_comments') . '/js/reformatPager.js');
    drupal_add_js(drupal_get_path('module', 'inline_comments') . '/js/loadPager.js');
    drupal_add_js(drupal_get_path('module', 'inline_comments') . '/js/reply.js');
    //      drupal_add_js(drupal_get_path('module', 'inline_comments') . '/js/charCount.js');
    drupal_add_js(drupal_get_path('module', 'inline_comments') . '/js/ajaxPoster.js');
    drupal_add_js(drupal_get_path('module', 'inline_comments') . '/js/comment_toggle.js');
    $this->query->add_field('node', 'type');
    $this->query->add_field('node', 'uid');
    $this->query->add_field('node_comment_statistics', 'comment_count');
  }
  function render($values) {
      global $user;
      $output = '<div class="node-nid">' . $values->nid . '</div>';
      $output .= '<div class="user-uid">' . $user->uid . '</div>';
      if (user_access('access comments')) {
        switch ($values->node_comment_statistics_comment_count) {
          case 0:
            $output .=  l(t('0 Comments'), 'node/' . $values->nid, array('attributes' => array('class' => 'comment-click')));
          break;
          case 1:
            $output .= l(t('View 1 Comment'), 'node/' . $values->nid, array('attributes' => array('class' => 'comment-click')));
          break;
          default:
            $numcomments = array('@numcomments' => $values->node_comment_statistics_comment_count);
            $string = t('View @numcomments Comments', $numcomments);
            $output .= l($string, 'node/' . $values->nid, array('attributes' => array('class' => 'comment-click')));
          break;
        }
      }

      if (user_access('skip comment approval')) {
        $output .= l(t('Add Comment'), 'comment/reply/' . $values->nid,  array('attributes' => array('class' => 'comment-link')));
      }
    return $output;
  }
}