<?php

/**
 * Builds the note view.
 */
class ViewNote {
	
	/**
	* The note object wrapped by this view
	*
	* @var DMNote
	*/
	protected $note;

	function __construct($note) {
		$this->note = $note;
	}
	
	function view() {
		$view = '';
		$view .= '<div class="note">';
		$view .= '<div class="text">' . preg_replace('/\n/', '<br>', check_plain($this->note->text())) . '</div>';
		
		$link = $this->note->attachmentLink();
		if($link) {
			$view .= '<div class="attachment">' . t('Attachment: ') . $this->note->attachmentLink() . '</div>';
		}
		
		$view .= '<div class="caption">' . t('Added ') . check_plain($this->note->createdDateTime()) . ' by ' . check_plain($this->note->creator()) . '</div>';
		$view .= '</div>';
		return $view;
	}


	function searchResult() {
		$html = '';
		
		$title = $this->note->typeLabel();
		$id = $this->note->pidNumber();
		
		$document = $this->note->document();
		
		if (!empty($document)) {
			$html = $this->documentSearchResult($title, $id, $document);
		}
		
		else {
			$purchase_order = $this->note->purchaseOrder();
			
			if (!empty($purchase_order)) {
				$html = $this->purchaseOrderSearchResult($title, $id, $purchase_order);
			}
		}
		
		return $html;	
	}
	
	function purchaseOrderSearchResult($title, $id, $purchase_order) {
		$purchase_order->load();
		$view = new ViewPurchaseOrder($purchase_order);
		return $view->searchResultWithTitle($title, $id);
	}
	
	function documentSearchResult($title, $id, $document) {
		$document->load();
		$view = new ViewDocument($document);
		$url = $document->viewerLinkUrl();
		$thumbnailUrl = $document->thumbnailUrl();
		return $view->searchResultWithTitle($title, $id, $url, $thumbnailUrl);
	}
}
	