<?php 

module_load_include('inc', 'Fedora_Repository', 'api/fedora_utils');

/*
 * Connect to the purchase order service, and retrieve information 
 * for the given $po_number. Returns a SimpleXmlElement.
 */
function fetch_uis_po($po_number) {	
	$base_url = variable_get('islandora_dm_po_service_url', null);
	$user = variable_get('islandora_dm_po_service_user', null);
	$pass = variable_get('islandora_dm_po_service_pass', null);
	
	$url = $base_url . '/' . $po_number;
	
	try {
		$xml_data = curl_fetch($url, $user, $pass);
		$xml = new SimpleXmlElement($xml_data);
	}
	
	catch (Exception $e) {
		$message = t('Unable to retrieve information for purchase order @p: @e', array('@p' => $po_number, '@e' => $e->getMessage()));
		watchdog(Constants::MODULE_NAME, $message, null, WATCHDOG_ERROR);
		$xml = null;
	}

	return $xml;
}

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