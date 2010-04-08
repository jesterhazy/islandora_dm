<?php

/**
 * Define menu items for this module
 */
function islandora_dm_menu() {
  $items = array();

  // admin page
  $items['admin/settings/islandora_dm'] = array(
    'title' => 'Islandora Document Management',
    'description' => 'Configure the Islandora Document Management module',
    'page callback' => 'drupal_get_form',
    'page arguments' => array('islandora_dm_admin'),
    'access arguments' => array('admin islandora_dm'),
  );
  
  $items['admin/settings/islandora_dm/settings'] = array(
    'type' => MENU_DEFAULT_LOCAL_TASK,
    'weight' => -1,
    'title' => 'Settings',
    'access arguments' => array('admin islandora_dm'),
  );
  
  $items['admin/settings/islandora_dm/install_objects'] = array(
    'type' => MENU_LOCAL_TASK,
    'title' => 'Install fedora objects',
    'description' => 'Install the fedora objects used by this module.',
    'page callback' => 'islandora_dm_install_objects',
    'access arguments' => array('admin islandora_dm'),
  );

  // home page
  $items['islandora_dm'] = array (
      'title' => 'Islandora Document Management',
      'page callback' => 'islandora_dm_home',
      'access arguments' => array('access islandora_dm'),
  );
  
  // about page
  $items['islandora_dm/about'] = array (
      'title' => 'About',
      'page callback' => 'islandora_dm_about',
      'access arguments' => array('access islandora_dm'),
  );
  
  // import documents
  $items['islandora_dm/import'] = array (
      'title' => 'Import Documents',
      'page callback' => 'islandora_dm_import',
      'access arguments' => array('import islandora_dm'),
  );
  
  // purchase order search
  $items['islandora_dm/po/search'] = array (
      'title' => 'Search',
      'page callback' => 'islandora_dm_po_search_form',
      'access arguments' => array('access islandora_dm'),
  );
  
  // purchase order view
  $items['islandora_dm/po/%/view'] = array (
      'type' => MENU_CALLBACK,
      'title' => 'Purchase Order !id',
      'title arguments' => array('!id' => 2),
      'page callback' => 'islandora_dm_po_view',
      'page arguments' => array(2),
      'access arguments' => array('access islandora_dm'),
  );
  
  // example fedora retrieval
  // TODO delete me
  $items['islandora_dm/fedora/%/view'] = array (
      'type' => MENU_CALLBACK,
      'title' => 'Basic Fedora View Test',
      'page callback' => 'islandora_dm_fedora_view',
      'page arguments' => array(2),
      'access arguments' => array('access islandora_dm'),
  );
  
  return $items;
}

?>