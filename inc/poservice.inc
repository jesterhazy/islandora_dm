<?php 

module_load_include('inc', 'Fedora_Repository', 'api/fedora_utils');

/*
 * Connect to the purchase order service, and retrieve information 
 * for the given $po_number. Returns a SimpleXmlElement.
 */
function fetch_uis_po($po_number) {
  $url = variable_get('islandora_dm_po_service_url', null);
  $user = variable_get('islandora_dm_po_service_user', null);
  $pass = variable_get('islandora_dm_po_service_pass', null);
  
  try {
    $xml_data = curl_fetch($url, $user, $pass);
    $xml = new SimpleXmlElement($xml_data);
  }
  
  catch (Exception $e) {
    $message = 'Unable to retrieve information for purchase order ' . $po_number;
    watchdog(t('islandora_dm'), t($message . ': ' . $e->getMessage()), null, WATCHDOG_ERROR);
    $xml = null;
  }

  return $xml;
}

?>