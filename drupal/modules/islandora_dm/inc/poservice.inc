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
