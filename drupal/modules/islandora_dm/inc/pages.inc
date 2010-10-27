<?php

function islandora_dm_home() {
  return unimplemented_page('home');
}

function islandora_dm_about() {  
  return unimplemented_page('about');
}

function islandora_dm_search() {  
  $page = '';
  
  $page .= drupal_get_form('islandora_dm_search_form');
    
  if (isset($_REQUEST['terms']) && !empty($_REQUEST['terms'])) {
    $terms = $_REQUEST['terms'];
    
    $page .= '<h3>' . t('Search Results') . '</h3>';
    
    // TODO paging, security
    $search = Search::searchByTerms($terms);
    $page .= $search->getSearchResultsView();
  
  }
  return $page;
}

function islandora_dm_search_form() {  
  $form = array();
    
  $form['terms'] = array(
        '#type' => 'textfield',
        '#title' => t('Search terms'),
        '#attributes' => array('placeholder' => 'search terms'),
        '#default_value' => isset($_REQUEST['terms']) ? $_REQUEST['terms'] : '',
        '#required' => TRUE
  );

  $form['submit'] = array(
      '#type' => 'submit',
      '#name' => 'search',
      '#value' => t('Search')
  );

  return $form;
}

function islandora_dm_search_form_submit($form, &$form_state) {
  $form_state['redirect'] = array('islandora_dm/search', 'terms=' . t($form_state['values']['terms']));
}

function islandora_dm_search_owner_mine() {
  global $user;
  $search = Search::searchByUser($user->name);  
  return $search->getSearchResultsView();
}

function islandora_dm_search_owner_unassigned() {
  $search = Search::searchByUser('unassigned');
  return $search->getSearchResultsView();
}

function islandora_dm_search_status_processed() {
  $search = Search::searchByStatus('processed');
  return $search->getSearchResultsView();
}

function islandora_dm_po_view($po_number) {
  return DMPurchaseOrder::fetchByNumber($po_number)->getView();
}

function unimplemented_page($name) {
  $output = <<<END
<p>todo: implement $name page</p>
END;

  return $output;
}
