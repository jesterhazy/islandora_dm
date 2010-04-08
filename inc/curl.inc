<?php 

/*
 * this could be merged with fedora_utils.inc::do_curl
 * if the drupal/fedora username logic was factored out of that method
 */
function curl_fetch($url, 
  $user = null, 
  $pass = null, 
  $return_to_variable = 1, 
  $number_of_post_vars = 0, 
  $post = null) {

    if (!function_exists("curl_init")) {
      throw new Exception('no curl support');
    }
    
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, FALSE);
    curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, FALSE);
    curl_setopt($ch, CURLOPT_FAILONERROR, TRUE); // Fail on errors
    curl_setopt($ch, CURLOPT_FOLLOWLOCATION, 1); // allow redirects
    curl_setopt($ch, CURLOPT_TIMEOUT, 90); // times out after 90s
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, $return_to_variable); // return into a variable

    curl_setopt($ch, CURLOPT_URL, $url);
    
    if (!empty($user) && !empty($pass)) {
      curl_setopt($ch, CURLOPT_USERPWD, "$user:$pass");
    }
    
    if ($number_of_post_vars>0 && $post) {
        curl_setopt($ch, CURLOPT_POST, $number_of_post_vars);
        curl_setopt($ch, CURLOPT_POSTFIELDS, "$post");
    }

    return curl_exec($ch);
}

?>