<?php

/**
 * @file
 * Default theme implementation to display a term.
 *
 * Available variables:
 * - $name: the (sanitized) name of the term.
 * - $content: An array of items for the content of the term (fields and
 *   description). Use render($content) to print them all, or print a subset
 *   such as render($content['field_example']). Use
 *   hide($content['field_example']) to temporarily suppress the printing of a
 *   given element.
 * - $term_url: Direct url of the current term.
 * - $term_name: Name of the current term.
 * - $classes: String of classes that can be used to style contextually through
 *   CSS. It can be manipulated through the variable $classes_array from
 *   preprocess functions. The default values can be one or more of the following:
 *   - taxonomy-term: The current template type, i.e., "theming hook".
 *   - vocabulary-[vocabulary-name]: The vocabulary to which the term belongs to.
 *     For example, if the term is a "Tag" it would result in "vocabulary-tag".
 *
 * Other variables:
 * - $term: Full term object. Contains data that may not be safe.
 * - $view_mode: View mode, e.g. 'full', 'teaser'...
 * - $page: Flag for the full page state.
 * - $classes_array: Array of html class attribute values. It is flattened
 *   into a string within the variable $classes.
 * - $zebra: Outputs either "even" or "odd". Useful for zebra striping in
 *   teaser listings.
 * - $id: Position of the term. Increments each time it's output.
 * - $is_front: Flags true when presented in the front page.
 * - $logged_in: Flags true when the current user is a logged-in member.
 * - $is_admin: Flags true when the current user is an administrator.
 *
 * @see template_preprocess()
 * @see template_preprocess_taxonomy_term()
 * @see template_process()
 */
?>
<div id="taxonomy-term-<?php print $term->tid; ?>" class="<?php print $classes; ?>">

 
  <?php
	$tids=arg(2);
	$nip='';
	$i=array(); 
	$j=0;
	if($tids==17){
	$all= taxonomy_get_children($tids);
	
	foreach ($all as $key => $value) {
		$ctid=$value->tid;
		$nip.= '<div class="term-news"><div class="term-news-title"><a href="/taxonomy/term/'.$ctid.'">'.$value->name.'</a></div>';
		$g = taxonomy_select_nodes($ctid, $pager = TRUE, $limit = 5, $order = array('t.sticky' => 'DESC', 't.created' => 'DESC'));
			foreach ($g as $key => $values) {
			$n=node_load($values);
			if(($n->promote)==1&&($n->sticky)==1){
			$i[$j]=$n;
			$j++;
			}
			//dsm($n);
			$ni= theme('image_style', array('style_name' => 'term-logo', 'path' => $n->field_image['und'][0]['uri'], 'alt' => $n->title));
			$nip.= '<div class="sub-term-news">'.$ni.'<a href="/node/'.$n->nid.'">'.$n->title.'</a></div>';
			
			}
		$nip.= '</div>';
		
		}
		
		
		print '<div class="top-news">'.render(node_view($i[0], 'default')).'</div>'.$nip;
	 
	 
	}
	?>
</div>
