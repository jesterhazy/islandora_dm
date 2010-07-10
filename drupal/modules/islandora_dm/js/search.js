$(document).ready(function() {
  $('div.clickable').click(function() {
      window.location = $(this).find('a:first').attr('href');
      event.preventDefault();
  });
  
  $('div.clickable').addClass('active');
});

