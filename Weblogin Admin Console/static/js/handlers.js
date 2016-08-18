/*---------*/
/* Globals */
/*---------*/
var debug = true;
var stream_error = false;
var entityMap = {
    "&": "&amp;",
    "<": "&lt;",
    ">": "&gt;",
    '"': '&quot;',
    "'": '&#39;',
    "/": '&#x2F;'
};

/*------------------*/
/* Helper functions */
/*------------------*/

/* Function   : is_spinner_visible
   Description:
    - Check if the clusters spinner is on right now
*/
function is_spinner_visible() {
  var res = $('#clusters_spinner').is(':visible');
  console.log("Cluster visible: " + res);
  return res;
};


/* Function   : escapeHtml
   Description:
    -  Escape a string containing HTML so it can be displayed consistently
*/
function escapeHtml(string) {
  return String(string).replace(/[&<>"'\/]/g, function (s) {
    return entityMap[s];
  });
};

function JSONstringify(json) {
    if (typeof json != 'string') {
        json = JSON.stringify(json, undefined, '\t');
    }

    var
        arr = [],
        _string = 'color:green',
        _number = 'color:darkorange',
        _boolean = 'color:blue',
        _null = 'color:magenta',
        _key = 'color:red';

    json = json.replace(/("(\\u[a-zA-Z0-9]{4}|\\[^u]|[^\\"])*"(\s*:)?|\b(true|false|null)\b|-?\d+(?:\.\d*)?(?:[eE][+\-]?\d+)?)/g, function (match) {
        var style = _number;
        if (/^"/.test(match)) {
            if (/:$/.test(match)) {
                style = _key;
            } else {
                style = _string;
            }
        } else if (/true|false/.test(match)) {
            style = _boolean;
        } else if (/null/.test(match)) {
            style = _null;
        }
        arr.push(style);
        arr.push('');
        return '%c' + match + '%c';
    });

    arr.unshift(json);

    console.log.apply(console, arr);
    return json;
}

/*------------------------*/
/* Socket events handlers */
/*------------------------*/


/*>>>>>>>>>>>>>>>>*/
/* NAMESPACE: ALL */
/*>>>>>>>>>>>>>>>>*/

/* Event: handler_connect */
function handler_connect() {
  console.log("Event: `connect` to " + current_socket)
};


/* Event: handler_disconnect */
function handler_disconnect() {
  console.log("Event: `connect` to " + current_socket)
};

function expire_session(expires_in, notify, message, reset_on){
    if (expires_in == 'undefined'){
        return false;
    }
    var time_left = expires_in
    var tick_duration = 1;
    var interval = setInterval(function(){
        time_left = time_left - tick_duration;
        if (time_left < 50){
            if (notify != 'undefined'){
                notify.text(time_left + ' seconds left');
            };
        };
        console.log('Session timeout in ' + time_left + ' seconds left');
    }, 1000);
    var timeout = setTimeout(handler_session_expired, time_left * 1000);
    function handler_session_expired(){
        clearInterval(interval);
        alert(message);
        window.location = Flask.url_for('ui.logout');
        // window.location = Flask.url_for('login');
    };
    if (reset_on != 'undefined'){
        reset_on.click(function(){
            console.log("Timer reset.");
            time_left = expires_in;
        });
    };
};

/* Event: Session timeouts */
$(function(){

    // Timeout: inactivity
    // var session_inactive = $(".session_inactive").text();
    // var session_inactive_msg = 'Your session has expired due to inactivity. Please login again.';
    // expire_session(session_inactive, $(".session_inactive_remaining"), session_inactive_msg, $(document.body));

    // Timeout: session closed
    var session_exp_time = $(".session_end").text();
    var current_time = $.now()/1000;
    var session_end = Math.trunc(session_exp_time - current_time);
    if (session_exp_time == 'undefined'){
        session_end = 'undefined';
    }
    var session_end_msg = 'Your session has been closed. Please login again.';
    expire_session(session_end, 'undefined', session_end_msg, 'undefined' );
});

/*>>>>>>>>>>>>>>>>>>>>>>*/
/* NAMESPACE: /weblogic */
/*>>>>>>>>>>>>>>>>>>>>>>*/

/* Event      : `receive_clusters`
   Namespace  : /weblogic
   Description:
    - Receive list of clusters + domain from server
    - Add clusters under appropriate section
*/
function handler_receive_clusters(data){
  console.log("\tEvent: `receive_clusters`");
  var clusters = data.clusters;
  var dom = data.domain;
  for (var id = 0; id < clusters.length; id++){
    var div = "<div class='dropdown_clusters dropdown_item'>" + clusters[id] + "</div></br>";
    $("#clusters_" + dom).append(div);
    console.log("\t\tCluster added: " + clusters[id])
  }
}

/* Event      : `begin_stream`
   Namespace  : /weblogic
   Description:
    - Define datatable
    - Show parse spinner
*/
function handler_begin_stream(){
  console.log('Event: `begin_stream`');
  stream_error = false;
  $('.parse').show();
  $('#parse_spinner').show();

  // DATATABLE DEFINITION
  $('#parse_table > thead').append(
  "\
    <tr>\
      <th>Environment</th>\
      <th>Domain</th>\
      <th>Cluster</th>\
      <th>Action</th>\
      <th>Status</th>\
      <th>Logfile</th>\
    </tr>\
  "
  );
  dataTable = $(".tablesorter").DataTable({
    "jQueryUI": true,
    "paginate": true,
    "scrollCollapse": true,
    "bAutoWidth": false,
    "bRetrieve": true,
    "search": {
      "regex": true
    },
    "createdRow": function( row, data, dataIndex ) {
      var sindex = 4;
      var lindex = 5;
      var status_cell = data[sindex];
      var logfile_cell = data[lindex];

      // Colorize status cells according to their value
      if ( status_cell == "ALIVE" || status_cell == "UP" || status_cell == "OK" )
      {
        $('td', row).eq(sindex).css('background-color', '#00ff00'); // green
      }
      else if ( status_cell == "DOWN" || status_cell == "FAIL" )
      {
        $('td', row).eq(sindex).css('background-color', '#ff0000'); // red
      }

      // Transform logfile cells to links
      if (logfile_cell) {
        $('td', row).eq(lindex).html("<a href="+logfile_cell + "?format=html>" + "View" + "</a>");
      }
      else { // hide column
        dataTable.column(lindex).visible(false);
      }
    },
    "lengthMenu": [[50, 100, 200, -1], [50, 100, 200, "All"]],
    buttons: [
      {
        extend: 'collection',
        text: 'Export',
        buttons: [ 'excelHtml5', 'pdfHtml5', 'copyHtml5'],
      }
    ]
  });
  $(".dataTables_wrapper").hide();
  $(dataTable.buttons().container()).appendTo('.dataTables_wrapper');
};

/* Event      : `api_response`
   Namespace  : /weblogic
   Description:
    - Add API response to dataTable
    - Slidedown dataTable
*/
function handler_api_response(data){
  console.log("\tEvent: `api_response`");
  console.log(data);
  $.each(data, function( i, row ){
    dataTable.row.add(row).draw();
  });
  $(".dataTables_wrapper").slideDown(1000);
};

/* Event      : `end_stream`
   Namespace  : /weblogic
   Description:
    - Hide spinner
*/
function handler_end_stream(){
  console.log('Event: `end_stream`');
  $("#parse_spinner").hide();
};


/*-----------------------*/
/* Click events handlers */
/*-----------------------*/

/* Event      : `click` on `dropdown_item`
   Description:
    - Mark clicked item as 'selected'
*/
function handler_enable_click(){
  console.log($(this).html().trim() + " clicked.");
  if ($(this).hasClass('selected')){
    $(this).toggleClass('selected');
  }
  else{
    $(this).addClass('selected');
  };
};


/* Event      : `click` on `dropdown_domain`
   Description:
    - Create / Delete a section labeled after domain
    - Emit `get_clusters` event to server
*/
function handler_clusters(){
  var dom = $(this).html().trim();
  console.log("Request clusters");
  if ($('#clusters_' + dom).length){ // Remove section
    $('#clusters_' + dom + '_wrapper').remove();
  }
  else{ // Add section
    $('#clusters').append('<div id=clusters_' + dom + '_wrapper><div id=clusters_' + dom + '></div></div>');
    var div_title = "<h3><b>" + dom + "</b></h3>";
    $('#clusters_' + dom + '_wrapper').prepend(div_title);
    sw.emit('get_clusters', dom);
  };
};


/* Event: `click` on `select_all`
   Desription:
    - Add class 'selected' to all elements when clicking on checkbox
*/
function handler_select_all(){
  var checked = $(this).prop('checked');
  var $children = $(this).parent().find('.dropdown_item');
  if (checked) {
    $children.addClass('selected');
  }
  else {
    $children.removeClass('selected');
  }
};


/* Event      : `click` on `select_all` for domains
   Description:
    - Perform `handler_clusters` action for every item selected by the `select_all` checkbox
*/
function handler_select_all_domains(){
  $(".dropdown_domains").each(function(){
    handler_clusters.call($(this));
  });
};

function handler_auth_error(){
  alert();
}

/* Event      : `click` on `submit`
   Description:
    - Add selected elements to separate lists
    - Destroy dataTable if defined before
    - Send selected data to server
*/
function handler_submit_button(event){
  var dom_array = [];
  var environ_array = [];
  var action_array = [];
  var cluster_array = [];
  var stream_msg_id = 1;
  var parse_msg_id = 1;

  // Empty output table
  if (typeof dataTable != 'undefined'){
    console.log("Datatable destroyed.");
    dataTable.destroy();
  }
  $('#parse_table tbody tr').remove();
  $('#parse_table thead tr').remove();

  // Create array for actions from selected items
  $(".dropdown_actions").each(function(index, item){
    var $item = $(item);
    if ($item.hasClass('selected')) {
      action_array.push($item.html().trim());
    }
  });

  // Create array for environments from selected items
  $(".dropdown_environments").each(function(index, item){
    var $item = $(item);
    if ($item.hasClass('selected')) {
      environ_array.push($item.html().trim());
    }
  });

  // Create array of domains from selected items
  $(".dropdown_domains").each(function(index, item){
    var $item = $(item);
    if ($item.hasClass('selected')) {
      dom_array.push($item.html().trim());
    }
  });

  // Create array for environments from selected items
  $(".dropdown_clusters").each(function(index, item){
    var $item = $(item);
    if ($item.hasClass('selected')) {
      cluster_array.push($item.html().trim());
    }
  });

  // If nothing checked, don't run anything
  if (action_array.length == 0 || dom_array.length == 0 || environ_array.length == 0){
    $(".submit").effect("shake", { times: 2, distance: 3 });
    var $error_msg = $(".error_msg");
    var error_msg = "You need to select at least one of each of those: </br> - Action </br> - Environment </br> - Domain";
    $error_msg.empty();
    $error_msg.append(error_msg);
    $(this).tooltip({
      items: ".submit",
      content: "You need to select at least one of each of those: </br> - Action </br> - Environment </br> - Domain",
      position: {my: "left+15 center", at: "right center"}
    });
    $(this).tooltip('enable');
    $(this).tooltip('open');
    setTimeout(function(){
      $(this).tooltip({ hide: {effect: "slideUp", duuration: 1000}});
      $(this).tooltip('disable');
    },2000);
    return false;
  }

  // Show / Hide output text / spinners
  $('.streaming > p').empty(); // Clear textboxes
  $('.stream').show();
  $('.parse').hide();
  $('#stream_text').css({ "padding": "10px 20px 10px 20px" });
  $('.spinners').show();


  // Send data to websocket
  data = { actions: action_array, environs: environ_array, domains: dom_array, clusters: cluster_array  };
  console.log(data);
  sw.emit('api_query', data);
};


/* Event      : 'click' on 'roller'
   Description:
    - Toggle div when clicking on roller
*/
function handler_roller(e){
  e.preventDefault();
  $this = $(this);
  var selector = $this.next('div');
  $(selector).slideToggle();
};