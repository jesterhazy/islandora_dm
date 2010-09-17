<?php

/**
 * Define menu items for this module
 */
function islandora_dm_menu() {
  $items = array();

  // admin page
  $items['admin/settings/islandora_dm'] = array(
    'title' => 'Islandora document management',
    'description' => 'Configure the Islandora document management module.',
    'page callback' => 'drupal_get_form',
    'page arguments' => array('islandora_dm_admin'),
    'access arguments' => array(Constants::PERMISSION_ADMIN),
  );
  
  $items['admin/settings/islandora_dm/settings'] = array(
    'type' => MENU_DEFAULT_LOCAL_TASK,
    'weight' => -1,
    'title' => 'Settings',
    'access arguments' => array(Constants::PERMISSION_ADMIN),
  );
  
  $items['admin/settings/islandora_dm/jms'] = array(
    'type' => MENU_LOCAL_TASK,
    'title' => 'JMS settings',
    'description' => 'Configure settings for the JMS service.',
    'page callback' => 'drupal_get_form',
    'page arguments' => array('islandora_dm_admin_jms'),
    'access arguments' => array(Constants::PERMISSION_ADMIN),
  );
  
  $items['admin/settings/islandora_dm/objects'] = array(
    'type' => MENU_LOCAL_TASK,
    'title' => 'Fedora objects',
    'description' => 'Install the fedora objects used by this module.',
    'page callback' => 'drupal_get_form',
    'page arguments' => array('islandora_dm_admin_objects_form'),
    'access arguments' => array(Constants::PERMISSION_ADMIN),
  );

  // home page
  $items[Constants::MODULE_NAME] = array (
      'title' => 'Islandora document management',
      'page callback' => 'islandora_dm_home',
      'access arguments' => array(Constants::PERMISSION_VIEW),
  );
  
  // about page
  $items['islandora_dm/about'] = array (
      'title' => 'About',
      'page callback' => 'islandora_dm_about',
      'access arguments' => array(Constants::PERMISSION_VIEW),
  );
  
  // import documents
  $items['islandora_dm/import'] = array (
      'title' => 'Import Documents',
      'page callback' => 'islandora_dm_import',
      'access arguments' => array(Constants::PERMISSION_EDIT),
  );
  
  // search
  $items['islandora_dm/search'] = array (
      'title' => 'Search',
      'page callback' => 'islandora_dm_search',
      'access arguments' => array(Constants::PERMISSION_VIEW),
  );
  
  // my purchase orders
  $items['islandora_dm/search/owner/mine'] = array (
      'title' => 'My Purchase Orders',
      'page callback' => 'islandora_dm_search_owner_mine',
      'access arguments' => array(Constants::PERMISSION_VIEW),
      'weight' => 1,
  );
  
  // unassigned purchase orders
  $items['islandora_dm/search/owner/unassigned'] = array (
      'title' => 'Unassigned',
      'page callback' => 'islandora_dm_search_owner_unassigned',
      'access arguments' => array(Constants::PERMISSION_VIEW),
      'weight' => 2,
  );
  
  // processed
  $items['islandora_dm/search/status/processed'] = array (
      'title' => 'Processed',
      'page callback' => 'islandora_dm_search_status_processed',
      'access arguments' => array(Constants::PERMISSION_VIEW),
      'weight' => 3,
  );
  
  // purchase order view
  $items['islandora_dm/po/%/view'] = array (
      'type' => MENU_CALLBACK,
      'title' => 'Purchase Order !id',
      'title arguments' => array('!id' => 2),
      'page callback' => 'islandora_dm_po_view',
      'page arguments' => array(2),
      'access arguments' => array(Constants::PERMISSION_VIEW),
  );
  
  return $items;
}

?>