<?php
$name='';
$names=array();
$utf8_names=array();
/*
*Get the name argument of the request url and import $name to $names array 
*/
if(isset($_GET['name'])){
$name = $_GET['name'];
$names = explode(",",$name);
//print count($names);
/*
* convert the $names array item to utf8
*/


  if (strstr($_SERVER["HTTP_USER_AGENT"], "Chrome")){
      $utf8_names=$names;
 

  }else{
         foreach ($names as $k=>$v) {
    
          $utf8_names[$k] = iconv("GB2312","UTF8",$v);  
          }
}
// print_r($utf8_names);
/*
*catch user-points  list with views 
*/
$view_data=views_get_view_result('user_points_json','data');
//print $view_data->result;
//print_r($view_data);
//print_r($view_data[0]);
/*
*foreach utf8_names and get user-points according  utf8_names array
*/
$my_user_points=array();
for($i=0;$i<count($utf8_names);$i++){
  
 $utf8_names_value=$utf8_names[$i];
 //print $utf8_names_value;
   $my_user_points[$i]='none';
	foreach($view_data as $v){
                
		if($v->users_name==$utf8_names_value){
		$my_user_points[$i]= $v->userpoints_points;
		}
    
	}
}
print_r($my_user_points);
}else{

echo '非法访问';
}

?>