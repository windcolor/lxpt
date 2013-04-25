
<div id="taxonomy-term" >

 
  <?php
	// Get vid from machine_name
	$my_vocabulary = taxonomy_vocabulary_machine_name_load('interest');
	//dsm($my_vocabulary);
	$my_vid = $my_vocabulary->vid;
	

	// Build and execute the query 
	//$query = new EntityFieldQuery();
	//$query->entityCondition('entity_type', 'taxonomy_term')->propertyCondition('vid', $my_vid);
	//$result = $query->execute();
	$result = taxonomy_get_tree($my_vid);
	// Process the result
	//dsm($result);
		
	foreach ($result as $key => $value) {
	    //dsm($value);
        $ctid = 0;		
		$ctid = $value->tid;
		$cterm= taxonomy_term_load($ctid);
		print '<div class="tools-top-term-1"><a href="/taxonomy/term/'.$ctid.'">'.theme('image_style', array('style_name' => 'term-logo', 'path' => $cterm->field__logo['und'][0]['uri'])).'<span>'.$value->name.'</span></a></div>';
		}
		
		
		
	?>
</div>
