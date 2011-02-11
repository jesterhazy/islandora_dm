$(document).ready(function() {
	$('div.clickable').click(function() {
			a = $(this).find('a:first');
			href = a.attr('href');
			target = a.attr('target');
			if (target) {
				window.open(href, target)
			}
			
			else {
				window.location = href;
			}
			event.preventDefault();
	});

	$('div.clickable a[href]').click(function() {
			event.stopPropagation();
	});
	
	$('div.clickable').addClass('active');
});

