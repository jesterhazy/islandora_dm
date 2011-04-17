<?php

/*
 * DMNote - methods for retrieving, displaying, and manipulating notes.
 */
class DMNote extends DMObject {
	const PID_PREFIX = 'n-';
		
	protected $noteXml;
	
	function typeLabel() {
		return 'Note';
	}
	
	function pidPrefix() {
		return self::PID_PREFIX;
	}
	
	static function number2pid($number) {
		return self::pidBase() . $number;
	}
	
	static function pidBase() {
		return DMObject::pidBase() . self::PID_PREFIX;
	}
	
	function createFedoraObject() {
		parent::createFedoraObject();
		$this->addModel(Constants::PID_CMODEL_NOTE, FEDORA_MODEL_URI);
	}
	
	function text() {
		return $this->noteXml->text;
	}
	
	function creator() {
		return $this->noteXml->creator;
	}
	
	function attachmentLink() {
		return $this->datastreamLink(Constants::DSID_ATTACHMENT);
	}

	function purchaseOrder() {
		return new DMPurchaseOrder(DMObject::findPurchaseOrderPid($this));
	}
	
	function addNoteDatastream($text, $username) {
		$noteXml = '<note><text>' . $text . '</text><creator>' . $username . '</creator></note>';		
		$this->fedora->add_datastream_from_string($noteXml, Constants::DSID_NOTE, Constants::DSLABEL_NOTE, Constants::MIME_TYPE_XML, 'X');
	}
	
	function addAttachmentDatastream($file) {
		$label = $file->filename;
		$this->addDatastreamFromFile($file->filepath, Constants::DSID_ATTACHMENT, $label, NULL, 'M');
	}

	static function createNote($username, $po_number, $text, $file = '') {
		$po_pid = DMPurchaseOrder::number2pid($po_number);
		try {
				$obj = new DMNote();
				$obj->createFedoraObject();
				$obj->addMembership($po_pid);
				$obj->addNoteDatastream($text, $username);
				$obj->addAttachmentDatastream($file, $username);
			}
			catch (Exception $e) {
					watchdog(Constants::MODULE_NAME, 'Error while creating fedora object @pid: @e', array('@pid' => $obj->pid(), '@e' => $e->getMessage()), WATCHDOG_WARN);
					$obj = null; 
			}

			return $obj;
	}
	
	function load() {
		if ($this->loaded()) return;
		
		parent::load();
		$this->noteXml = $this->datastreamAsXml(Constants::DSID_NOTE);
	}
	
	static function findRelatedNotes($parent_pid) {
		$notes = array();
		
		$pids = DMObject::findRelatedPids($parent_pid, Constants::PID_CMODEL_NOTE);
		foreach($pids as $pid) {
			$notes[] = new DMNote($pid);
		}
		
		return $notes;
	}
}

/*
 * Drupal callback handlers
 */
function islandora_dm_add_note_form(&$form_state, $po_number = NULL, $document_number = NULL) {
	$form = array();
	$form['#attributes'] = array('enctype' => 'multipart/form-data');
	
	if ($po_number) {
		$form['po_number'] = array(
			'#type' => 'value',
			'#value' => $po_number,
		);
	}
	
	if ($document_number) {
		$form['document_number'] = array(
			'#type' => 'value',
			'#value' => $document_number,
		);
	}
	
	$form['note'] = array(
		'#type' => 'textarea',
		'#title' => t('Note'),
		'#required' => TRUE,
	);
	
	$form['attachment'] = array(
    '#title' => t('Add attachment'),
    '#type'  => 'file',
		'#description' => 'Attach a file to this note (optional).',
  );
  
	$form['submit'] = array(
		'#type' => 'submit',
		'#name' => 'save',
		'#value' => t('Save'),
	);

	$form['cancel'] = array(
		'#type' => 'submit',
		'#name' => 'cancel',
		'#value' => t('Cancel'),
	);

	return $form;
}

function islandora_dm_add_note_form_submit($form, &$form_state) {
	if ($form_state['clicked_button']['#name'] == 'save' ) {
		global $user;
				
		$number = $form_state['values']['document_number'];
		if ($number) {
			$type = 'document';
			$pid = DMDocument::number2pid($number);
		}
		
		else {
			$number = $form_state['values']['po_number'];
			$type = 'purchase order';
			$pid = DMPurchaseOrder::number2pid($number);
		}	
		
		$text = $form_state['values']['note'];
		$file = file_save_upload('attachment');
		
		$note = new DMNote();
		$note->createFedoraObject();
		$note->addMembership($pid);
		$note->addNoteDatastream($text, $user->name);
		
		if ($file != 0) {
			$note->addAttachmentDatastream($file);
		}
		
		drupal_set_message(t('Added note @n1 to @type @n2.', array('@n1' => $note->pidNumber(), '@type' => $type, '@n2' => $number)));
	}
	
	$url_part = ($form_state['values']['po_number']) 
		? '/po/' . $form_state['values']['po_number']
		: '/document/' . $form_state['values']['document_number'];
		
	$form_state['redirect'] = array('islandora_dm' . $url_part . '/view');
}

function islandora_dm_delete_note($note_number, $origin, $origin_number) {
	$note = new DMNote(DMNote::number2Pid($note_number));
	$note->load();
	
	// if this is last note attached to document, and document has no PO, we need to mark it as unlinked
	$document = $note->document();
	if ($document) {
		$document->load();
		$purchase_orders = $document->purchaseOrders();
		if (empty($purchase_orders)) {
			$notes = $document->notes();
			if (count($notes) <= 1) {
				$document->addMembership(Constants::PID_COLLECTION_PURCHASE_ORDERS_UNLINKED_DOCUMENTS);
			}
		}
	}
	
	// purge note
	$note->purge();
	drupal_set_message(t('Deleted note @n', array('@n' => $note_number)))	;
	
	drupal_goto('islandora_dm/' . $origin . '/' . $origin_number . '/view');
}