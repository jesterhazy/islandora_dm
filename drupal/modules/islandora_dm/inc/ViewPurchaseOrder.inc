<?php

/**
 * Builds the purchase order view and search result view.
 */
class ViewPurchaseOrder {
	
	/**
	 * The purchase order object wrapped by this view
	 *
	 * @var DMPurchaseOrder
	 */
	protected $purchaseOrder;
	
	function __construct($purchase_order) {
		$this->purchaseOrder = $purchase_order;
	}
	
	/**
	 * Build an array of table rows, suitable for theme_table.
	 */
	function purchaseOrderDetailsAsTableRows() {
		$rows = array(
			array(array('data' => t('PO Date:'), 'class' => 'label'), check_plain($this->purchaseOrder->poDate()), null),
			array(array('data' => t('Department:'), 'class' => 'label'), check_plain($this->purchaseOrder->department() . ' (' . $this->purchaseOrder->departmentId() .')'), null),
			array(array('data' => t('Supplier:'), 'class' => 'label'), check_plain($this->purchaseOrder->supplier() . ' (' . $this->purchaseOrder->supplierId() .')'), null),
			array(array('data' => t('Requisition:'), 'class' => 'label'), check_plain($this->purchaseOrder->requisitionNumber()), null),
			array(array('data' => t('Import Date:'), 'class' => 'label'), check_plain($this->purchaseOrder->createdDate()), null),
		);
		
		return $rows;
	}
	
	function buildPurchaseOrderTable() {
		return theme('table', null, $this->purchaseOrderDetailsAsTableRows());
	}
	
	function view() {
		$view = "\n\n";
		$view = '<div class="islandora-dm-view">';

		$this->addErrorMessages();

		// return an empty view (with errors) if fedora is empty.
		if (!$this->purchaseOrder->loaded()) {				
			return $view;
		}

		$view .= $this->addButtons();

		$view .= "\n\n";
		$view .= '<h3>Details</h3>';
		$view .= $this->buildPurchaseOrderTable();
		$view .= $this->addDocumentsView();
		$view .= $this->addNotesView();
		$view .= $this->addClassificationView();
		$view .= $this->addDispositionView();

		$view .= "</div>";		
		$view .= "\n\n";

		return $view;
	}
		
	function addButtons() {
		$view = "\n\n"; 

		if (user_access(Constants::PERMISSION_EDIT)) {
			$view .= '<div class="buttons">';

			$view .= $this->addButton('add_note', 'Add Note');

			if (!$this->purchaseOrder->synchedWithUIS()) {
				drupal_set_message(t('This record is out of date. Update before proceeding.'), Constants::DRUPAL_MESSAGE_WARNING);
				$view .= $this->addButton('update', 'Update from UIS');
			}

			$view .= '</div>';
		}

		return $view;
	}
		
	static function addButton($button_url, $button_label) {
		$button = '<div class="button"><a href="' . $button_url . '"><button>' . t($button_label) .'</button></a></div>';
		return $button;
	}
		
		function addDocumentsView() {
		$view = "\n\n";
		$view .= '<div class="documents"><h3>Documents</h3>';
		
		$documents = $this->purchaseOrder->documents(); 
		if (!empty($documents)) {
			foreach ($documents as $document) {
				$document->load();					
				$documentView = new ViewDocument($document);
				$view .= $documentView->embeddedView();
			}
			
			$view .= '<div class="clear"></div>';
		}
		else {
			$view .= '<p>' . t('No documents.') . '</p>';
		}

		$view .= '</div>';
		return $view;
	}
	
	function addNotesView() {
		$notes = $this->gatherNotes();
		return ViewNote::pageSectionView($notes);
	}
	
	function gatherNotes() {
		$notes = $this->purchaseOrder->notes();
		$documents = $this->purchaseOrder->documents();
		foreach($documents as $document) {
			$document->load();
			$notes = array_merge($notes, $document->notes());
		}
		
		return $notes;
	}
	
	function addErrorMessages() {
		// empty fedora is an error.
		if (!$this->purchaseOrder->loaded()) {
			drupal_set_message(t('Unable to retrieve purchase order ' . $this->purchaseOrder->number() . '.'), Constants::DRUPAL_MESSAGE_ERROR);
		}

		// empty uis data is a warning
		// only set if fedora was ok
		else if (!$this->purchaseOrder->uisLoaded()) {
			drupal_set_message(t('Unable to retrieve UIS data for purchase order ' . $this->purchaseOrder->number() . '.'), Constants::DRUPAL_MESSAGE_WARNING);
		}
	}
	
	function addDispositionView() {
		$view = "\n\n";

		$ds = $this->purchaseOrder->disposition();
		if ($ds) {
			$view .= '<h3>Disposition</h3>';

			$rows = array(
				array(array('data' => t('Disposition Action:'), 'class' => 'label'), check_plain($ds->dispositionAction), null),
				array(array('data' => t('Retention Period:'), 'class' => 'label'), check_plain($ds->retentionPeriod), null),
				array(array('data' => t('Disposition Date:'), 'class' => 'label'), check_plain($ds->dispositionDate), null),
				array(array('data' => t('Disposition Reason:'), 'class' => 'label'), check_plain($ds->dispositionReason), null),
				array(array('data' => t('Disposition Decision User:'), 'class' => 'label'), check_plain($ds->dispositionDecisionUser), null),
			);

			$view .= theme('table', null, $rows);
		}

		return $view;
	}

	function addClassificationView() {
		$view = "\n\n";

		$ds = $this->purchaseOrder->classification();
		if ($ds) {

			$view .= '<h3>Classification</h3>';

			$rows = array(
				array(array('data' => t('Office of Record:'), 'class' => 'label'), check_plain($ds->officeOfRecord), null),
				array(array('data' => t('Unit of Record:'), 'class' => 'label'), check_plain($ds->unitOfRecord), null),
				array(array('data' => t('Depositor of Record:'), 'class' => 'label'), check_plain($ds->depositorOfRecord), null),
				array(array('data' => t('Date of Deposit:'), 'class' => 'label'), check_plain($ds->dateOfDeposit), null),
				array(array('data' => t('Classification Code:'), 'class' => 'label'), check_plain($ds->classificationCode), null),
			);

			$view .= theme('table', null, $rows);
		}

		return $view;
	}
	
	function searchResult() {
		return $this->searchResultWithTitle('PO #', $this->purchaseOrder->number());
	}

	function searchResultWithTitle($title, $id) {
		$view = "\n\n";

		$this->addErrorMessages();

		// return an empty view (with errors) if fedora is empty.
		if (!$this->purchaseOrder->loaded()) {
			return $view;
		}

		$view .= '<div class="search-result clickable">';
		$view .= '<div class="title">' . t($title) . ' <a href="' . $this->purchaseOrder->linkUrl($this->purchaseOrder->pid()) . '">' . $id . '</a></div>';

		$documents = $this->purchaseOrder->documents();
		if (!empty($documents)) {
			$document = $documents[0];
			$document->load();					
			$documentView = new ViewDocument($document);
			$view .= $documentView->thumbnailForSearchResult($document->thumbnailUrl());
		}

		$view .= $this->buildPurchaseOrderTable();
		$view .= '</div>';

		return $view;
	}
} 