;(function($) {
  
  Drupal.behaviors.comment_toggle = {
    attach: function(context, settings) {
    $('.comment-click', context).unbind('click').bind('click', function() {
      $(this).loadComments();
      return false;
    });
  }
 };
 
 Drupal.behaviors.comment_reply = {
   attach: function(context, settings) {
    $('#comment-form', context).unbind('submit').bind('submit', function(e){
       var options = {};
       options['slideDown'] = false;
       options['targetEle'] = $(this).parents('.views-row').find('.comment-group');
       $('#comment-form', context).ajaxPost(options, function(){
           Drupal.attachBehaviors();
       });
       e.preventDefault();
     });
   }
 };
 
 Drupal.behaviors.comment_reply_link = {
   attach: function(context, settings) {
      $('.reply_link').click( function(e){
        e.preventDefault();
        comment_content = $(this).parents('.links').prev().html();
        submitted =  $(this).parents('.links').prev().prev().html();
        //html_comment = "<!-- Add your reply below this line  -->";
        quote = "<div class='comment_quote'>" + submitted  + comment_content + "</div>";
        re = /\n/gi;
        quote =  quote.replace(re, '');
        quote = quote + '\n';
        addcomment =  $(this).parents('.views-row').find('.comment-form').length;
        if(addcomment == 0){
          $(this).reply({
            content: quote
          });
        }
      });
   }
 };
 
 Drupal.behaviors.comment_pager = {
   attach: function(context, settings) {
    $('.pager-next a.active', context).click( function(e){
         $(this).loadPager();
         e.preventDefault();
     });
   }
 };
 
 Drupal.behaviors.comment_close_link = {
   attach: function(context, settings) {
     $('a.closelink').click(function(e) {
       var group =  $(this).parents('.comment-group');
       var theseComments = new closeComments(group);
       theseComments.group = $(this).parents('.comment-group');
       theseComments.slide();
       theseComments.scrollerUp();
       e.preventDefault();
     });
   }
 };

 Drupal.behaviors.comment_form_close_link = {
   attach: function(context, settings) {
    $('a.formcloselink').click(function(e) {
       var group =  $(this).parents('.comment-form');
       var thoseComments = new closeComments(group);
       thoseComments.slide();
       thoseComments.scrollerUp();
       group.remove();
       e.preventDefault();
     });
   }
 };
 
 Drupal.behaviors.comment_load_comment_form = {
   attach: function(context, settings) {
     $('a.comment-link, context').unbind('click').bind('click', function(){
        var options = {};
         // open expanded comments if not expanded already
         var row =  $(this).parents('.views-row');
         var commentclick = row.find('.comment-click');
         var commentgroup = row.find('.comment-group');
         var reply = row.find('.comment-form');
         if(commentclick){
           console.log(commentclick.text());
          if(commentgroup.is(':hidden') || commentgroup.length == 0){
            if(commentclick.text()!= '0 Comments'){
              commentclick.click();
            }
            else {
              // 0 comments so add comment group div
              row.append("<div class='comment-group'></div>");
            }
          }
         }
         if(reply.length == 0 ) {
           options['targetEle'] = $(this).parents('.views-row').find('.comment-group');
           $(this).reply();
         }
         return false;
     });
   }
 };
 
 
 function closeComments(group) {
   if(group) {
     this.group = group;
   }
   this.scroller = group.offset().top -150;
 }
 closeComments.prototype.slide = function(){
   $(this.group).slideUp('slow');
 };
 closeComments.prototype.scrollerUp = function(){
   $('html,body').animate({
     scrollTop: this.scroller
   },
   1000);
 };
})(jQuery);






