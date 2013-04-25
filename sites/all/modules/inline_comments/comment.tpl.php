<?php
?>
<div class="<?php print $classes . ' ' . $zebra; ?>"<?php print $attributes; ?>>
  <?php print $picture ?> 
   <span class="submitted"><?php print $submitted ?></span>
	   <span class="signature">
      <?php print $signature ?>
    </span>
  <div class="content"<?php print $content_attributes; ?>>
  <?php if ($new): ?>
    <span class="new"><?php print drupal_ucfirst($new) ?></span>
  <?php endif; ?>
    <?php hide($content['links']); print render($content); ?>
    <?php if ($signature): ?>
    <?php endif; ?>
  </div>


  <?php print render($content['links']) ?>
</div>
