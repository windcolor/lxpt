;(function($) {
$.fn.reply = function(options) {
	var defaults = {
		content: ''  
	};
	var theoptions = $.extend(defaults, options); 
	return this.each(function(index) {
		var $$ = $(this);
		//$('.comment-form').remove();
		var hreflink = $$.attr('href');
		var regex = /\/comment\/reply\/([0-9]+)/i;
		//var regex = /(comment)/
		var params = hreflink.match(regex);
		var theurl = '/ajax/inline_comments/get_comment_form/' + params[1]; //+ '/' + params[2];
		var row =  $$.parents('.views-row');
		row = $(row);
		var comment = row.find('.comment-group');
		comment = $(comment);
		$('<div class=\'comment-form\'></div>').insertBefore(comment).hide();
		Drupal.attachBehaviors(row);
		var cform = row.find('.comment-form');
		cform = $(cform);
		$.ajax({
			type: 'GET',
			url: theurl,
			success: function(res) {
        //textarea = $(res.data).find('.form-textarea-wrapper');
        //label = $(res.data).find('label[for="edit-comment-body-und-0-value"]');
				cform.append(res.data).show();
				cform.find('#edit-author').remove();
				cform.find('#edit-comment-body-und-0-format').remove();
				//cform.find('#comment-form').find('.form-textarea-wrapper').addClass('clearfix').prepend(label);
				//cform.prepend(theoptions.content);
				//var counter = $("<div>You have used <span class=\'totalcharsused\'>0</span> characters</div>");
				//cform.find('.description').append(counter);
				cform.find('#edit-comment-body').addClass('ajaxComments');
				var closeLink = $('<a href=\'#\' class=\'formcloselink\'>X Close</a>');
				cform.append(closeLink);
				Drupal.attachBehaviors(cform);
				//cform.find('#edit-comment').val('').keyup();
			  //cform.find('.description div').append(' including reply quotes');
			}
		});
	});
};
})(jQuery);