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
	
	function embeddedView() {
		$view = '';
		$view .= '<div class="note">';
		$view .= '<div class="text">' . preg_replace('/\n/', '<br>', check_plain($this->note->text())) . '</div>';
		
		$link = $this->note->attachmentLink();
		if($link) {
			$view .= '<div class="attachment">' . t('Attachment: ') . $this->note->attachmentLink() . '</div>';
		}
		
		$view .= '<div class="caption">';
		$view .= t('Added ') . check_plain($this->note->createdDateTime()) . ' by ' . check_plain($this->note->creator()) . '. ';
		$view .= '<a href="note/' . $this->note->pidNumber() . '/delete">Delete this note</a>.';
		$view .= '</div>';
		$view .= '</div>';
		return $view;
	}

	static function pageSectionView($notes) {
		$html = "\n\n";
		$html .= '<div class="notes"><h3>Notes</h3>';
		
		if (!empty($notes)) {
			foreach ($notes as $note) {
				$note->load();
				$view = new ViewNote($note);
				$html .= $view->embeddedView();
			}
		}
		else {
			$html .= '<p>' . t('No notes.') . '</p>';
		}

		$html .= '</div>';
		return $html;
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
		$url = $document->linkUrl();
		$thumbnailUrl = $document->thumbnailUrl();
		return $view->searchResultWithTitle($title, $id, $url, $thumbnailUrl);
	}
}
	