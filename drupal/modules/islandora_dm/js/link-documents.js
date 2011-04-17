$(document).ready(function() {
	$('#islandora-dm-import-link-documents-form input[type="radio"][name$="reftype"]').click(function() {
		radio = $(this);
		label = $(radio).parent().text().trim();
		target = $(radio).parent().parent().parent().parent().next().children('label');
		$(target).text(label + ' number(s):');
	});
});
