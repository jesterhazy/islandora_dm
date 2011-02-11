<?php

function unimplemented_page($name) {
	$output = <<<END
<p>todo: implement $name page</p>
END;

	return $output;
}

function islandora_dm_home() {
	return unimplemented_page('home');
}

function islandora_dm_about() {	
	return unimplemented_page('about');
}

function islandora_dm_po_view($po_number) {
	$pid = DMPurchaseOrder::number2pid($po_number);
	$purchase_order = new DMPurchaseOrder($pid);
	$purchase_order->load();
	$view = new ViewPurchaseOrder($purchase_order);
	return $view->view();
}

function islandora_dm_search() {	
	$page = '';
	
	$page .= drupal_get_form('islandora_dm_search_form');
		
	if (isset($_REQUEST['terms']) && !empty($_REQUEST['terms'])) {
		$terms = $_REQUEST['terms'];
		
		$search = Search::createSearchUsingTerms($terms, $offset); 
		$search->execute();
		$view = new ViewSearchResults($search->results());
		$page .= $view->view();
	
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

function islandora_dm_update_uis_data($po_number) {
	$pid = DMPurchaseOrder::number2pid($po_number);
	$purchase_order = new DMPurchaseOrder($pid);
	$purchase_order->load();
	$purchase_order->updateUISDatastream();
	drupal_set_message(t('Updated record from UIS'));
	drupal_goto('islandora_dm/po/' . $po_number . '/view');
}

