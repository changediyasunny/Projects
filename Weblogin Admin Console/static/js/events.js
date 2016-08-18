/*----------------------*/
/* JQuery UI Components */
/*----------------------*/
$(".rollers").button({
  icons: {
    secondary: "ui-icon-plusthick"
  },
});

$(".submit").button({
});

/*----------------*/
/* Sockets events */
/*----------------*/

// SocketIO for streaming command output
var sw = io('http://' + document.domain + ':' + location.port + '/weblogic');
current_socket = 'weblogic_socket'
sw.connect();
sw.on('connect', handler_connect);
sw.on('disconnect', handler_disconnect);
sw.on('clusters', function(data){ handler_receive_clusters(data) });
sw.on('begin_stream', handler_begin_stream);
sw.on('api_response', function(data){ handler_api_response(data) });
sw.on('end_stream', handler_end_stream);

var ss = io('http://' + document.domain + ':' + location.port + '/session')
ss.connect();
ss.on('connect', handler_connect)
ss.on('disconnect', handler_disconnect);

/*--------------*/
/* Click events */
/*--------------*/
$(".dropdown_wrapper").on('click', ".dropdown_item", handler_enable_click);
$(".dropdown_wrapper").on('click', ".dropdown_domains", handler_clusters);
$(".select_all").on("click", handler_select_all);
$(".select_all_domains").on("click", handler_select_all_domains);
$(".submit").on("click", function(event) { handler_submit_button(event) });
$(".rollers").on("click", handler_roller);
$(document).click(function(){
    sw.emit('ping');
});