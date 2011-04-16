<?php

module_load_include('php', 'islandora_solr_search', 'Solr/Service');

class Search {
  const MAX_RESULTS = 10;
  function searchPage() {
    $page = '<div class="islandora-dm-search-page">';
    $page .= drupal_get_form('islandora_dm_search_form');

    if (!isset($_REQUEST['x'])) {
      if ($this->hasQuery()) {
        $results = $this->executeSearch();
        $page .= $this->renderResults($results);
      }
    }
	  
    $page .= '</div>';
    return $page;
  }
  
  function hasQuery() {
  	return isset($_REQUEST['f0']) && isset($_REQUEST['s']) && isset($_REQUEST['t0']);
  }
  
  function buildSolrOptions() {
    // TODO this parameter should probably be pulled from solr block config
    $solr_options = array('qt' => 'standard');
    
    // set up sort
    $sort_options = array(
      '1' => NULL,
      '2' => 'islandora-dm.date.created desc',
      '3' => 'islandora-dm.date.modified desc',
      '4' => 'islandora-dm.sort.po.department asc',
      '5' => 'islandora-dm.sort.po.supplier asc',
      );
      
    $sort = $sort_options[$_REQUEST['s']];
    
    if ($sort) {
      $solr_options['sort'] = $sort;
    }
    
    return $solr_options;
  }
  
  function isSortOnPurchaseOrderField() {
    return in_array($_REQUEST['s'], array('4', '5'));
  }
  
  function buildSolrQuery() {
    $query = '';
  	
    // set up field
    $field_options = array(
      '1' => 'islandora-dm.text',
      '2' => 'PID',
      '3' => 'islandora-dm.query.po.department',
      '4' => 'islandora-dm.query.po.supplier',
      '5' => 'islandora-dm.query.po.requisition',
      );

    $tn = (int) $_REQUEST['tn'];
    for ($i = 0; $i <= $tn; $i++) {
      $t = $_REQUEST['t' . $i];
      if (!empty($t)) {
        $f = $field_options[$_REQUEST['f' . $i]];
        
        if ($f == 'PID') {
          $t = DMPurchaseOrder::number2pid($t);
        }

        // set up term. 
        // allow for space-delimited multiple terms even though these 
        // should only be used on searches against islandora-dm.text 
        $split = split(' ', $t);
        $joined = join(" and ", $split);
        $term = Apache_Solr_Service::escape($joined);

        if ($i > 0) {
          $query .= ' ';
        }

        $query .= '+' . $f . ':(' . $term . ')';
      }
    }
        
    if ($this->isSortOnPurchaseOrderField()) {
    	$query .= ' and rels.hasModel:(' . Apache_Solr_Service::escape(Constants::PID_CMODEL_PURCHASE_ORDER) . ')';
    }

    return $query;
  }
  
  function offset() {
    return $_REQUEST['o'] ? $_REQUEST['o'] : 0;
  }
  
  function executeSearch() {
    $query = $this->buildSolrQuery();
    $options = $this->buildSolrOptions();

    $solr = $this->getSolrService();
    
    $results = $solr->search($query, $this->offset(), self::MAX_RESULTS, $options);
    
    return $results;
  }
  
  static function queryStringForOffset($offset) {
    $qs = '?s=' . urlencode($_REQUEST['s']);
    $qs .= '&o=' . $offset;

    $rows = (int) $_REQUEST['tn'];
    $qs .= '&tn=' . $rows;

    for ($i = 0; $i <= $rows; $i++) {
      $qs .= '&f' . $i . '=' . urlencode($_REQUEST['f' . $i]);
      $qs .= '&t' . $i . '=' . urlencode($_REQUEST['t' . $i]);
    }

    return $qs;
  }
  
  /**
   * Prepare a solr service instance for use.
   */
  function getSolrService() {
    $solr_url = variable_get(Constants::SETTING_SOLR_URL, Constants::SETTING_SOLR_URL_DEFAULT);
    $parts = parse_url($solr_url);
    
    $solr = new Apache_Solr_Service($parts['host'], $parts['port'], $parts['path']);
    return $solr;
  }
  
  function renderResults($results) {
  	$view = new ViewSearchResults($results, $this->offset());
  	return $view->view();
  }

  function searchForm() {
    $form = array();
    
    $field_options = array(
      '1' => 'All fields',
      '2' => 'PO Number',
      '3' => 'Department',
      '4' => 'Supplier',
      '5' => 'Requisition',
      );
      
    $sort_options = array(
      '1' => 'Relevance',
      '2' => 'Imported date',
      '3' => 'Updated date',
      '4' => 'Department',
      '5' => 'Supplier',
      );

    $rows = (int) $_REQUEST['tn'];
    $j = 0;
    for ($i = 0; $i <= $rows; $i++, $j++) {
      if (($i > 0) and !isset($_REQUEST['x']) and empty($_REQUEST['t' . $i])) {
        $j--;
        continue;
      }

      if ($i > 0) {
        $form['br' . $j] = array('#value' => '<br>');
      }

      $form['f' . $j] = array(
        '#type' => 'select',
        '#options' => $field_options,
        '#default_value' => isset($_REQUEST['f' . $i]) ? $_REQUEST['f' . $i] : '1',
        '#required' => TRUE);

      $form['t' . $j] = array(
        '#type' => 'textfield',
        '#title' => 'Search terms',
        '#attributes' => array('placeholder' => 'search terms'),
        '#default_value' => isset($_REQUEST['t' . $i]) ? $_REQUEST['t' . $i] : '',
        '#size' => 30,
        '#required' => ($i == 0 && !isset($_POST['addrow'])));

      if ($i == 0) {
        $form['submit'] = array(
          '#type' => 'submit',
          '#name' => 'search',
          '#value' => t('Search'));

        $form['s'] = array(
          '#type' => 'select',
          '#options' => $sort_options,
          '#default_value' => isset($_REQUEST['s']) ? $_REQUEST['s'] : '1',
          '#required' => TRUE);
      }
    }
    $form['br'] = array('#value' => '<br>');

    $form['addrow'] = array(
      '#type' => 'submit',
      '#name' => 'addrow',
      '#value' => t('Add'));	  

    $form['tn'] = array(
      '#type' => 'hidden',
      '#value' => $j - 1);

    return $form;
  }
  
  function submitForm(&$form_state) {
    $tn = (int) $form_state['values']['tn'];
    
    $params = array(
        's' => t($form_state['values']['s'])
    );

    for ($i = 0; $i <= $tn; $i++) {
        $params['f' . $i] = t($form_state['values']['f' . $i]);
        $params['t' . $i] = t($form_state['values']['t' . $i]);
    } 

    if ($form_state['clicked_button']['#name'] == 'addrow') {
      $tn++;
      $params['x'] = 1;
    }

    $params['tn'] = $tn;

    $form_state['redirect'] = array('islandora_dm/search', $params);
  }
}

function islandora_dm_search_page() {	
  $search = new Search();
  return $search->searchPage();
}

function islandora_dm_search_form() {
  $search = new Search();
  return $search->searchForm();
}

function islandora_dm_search_form_submit($form, &$form_state) {
  $search = new Search();
  $search->submitForm($form_state);
}
