
/*
Function: Loads on Document Start
Usage: Load the table for first environment.
*/
$(function(){
    env_select('dev0');
})

/*
Function: Dynamically create Rows for Table
Usage: To add dynamic data & rows to table tbody.
*/
function create_table_rows(data)
{
  var fields = JSON.parse(data.fields);
  var my_env = data.my_env;
  var DISP_FLAG = data.DISP_FLAG;
  var SVN_MSG = data.SVN_MSG;

  if(data.hasOwnProperty('env_list'))
  {
    var env_list = JSON.parse(data.env_list);
  }
  var all_rows = '';
  $.each(fields, function(K, V){

      var new_row = "\
            <tr>\
                <td style='width:45px;'><input type='checkbox' name='" + K + "'/>&nbsp;</td>\
                <td contenteditable='true'>" + K + "</td>\
                <td contenteditable='true'>" + V + "</td>\
            </tr>\ ";

      all_rows = all_rows + new_row;
  });
  
  $("#dataTable tbody").html(all_rows);
  $('#myenv_id').text(my_env);
  $('#msg-disp').text(SVN_MSG);
  console.log("Create-Table-Rows: Rows Created & Added to Table.")
}


/*
Function: Display properties of Environment
Usage: get properties of environment & add table rows

*/
function env_select(dom_name)
{
  console.log("Send-Menu-Click: Sending Env for properties." + dom_name)

  $.ajax({
    type: 'POST',
    contentType: 'application/json;charset=UTF-8',
    data: JSON.stringify(dom_name, null, '\t'),
    url: "/settings/my_index",
    success: function (data){
      console.log("Success:Logging all data now..."+data);
      create_table_rows(data);
    }
  });
}


function make_blur(t_index)
{
  t_index.innerHTML = "";
}

/*
Function: 
*/
function old_add_row()
{
  var myTable = document.getElementById("dataTable");
  var new_row = myTable.insertRow(1);

  var x = new_row.insertCell(0);
  x.innerHTML = "<input type='checkbox' name='chk'/>"
  x.setAttribute("type","checkbox");

  var cell1 = new_row.insertCell(1);
  var cell2 = new_row.insertCell(2);

  cell1.innerHTML = "Enter Here";
  cell2.innerHTML = "Enter Here";
  cell1.setAttribute("contenteditable","true");
  cell2.setAttribute("contenteditable","true");

}


/*
Function: Create Row & attributes
Usage: Creates row specific to table & add after tbody
*/
function add_row()
{
  console.log("add_row(): creating rows.");  
  var myTable = document.getElementById("dataTable_tbody");
  var new_row = myTable.insertRow(0);

  var x = new_row.insertCell(0);
  x.innerHTML = "<input type='checkbox' name='chk'/>"
  x.setAttribute("type","checkbox");

  var cell1 = new_row.insertCell(1);
  var cell2 = new_row.insertCell(2);

  cell1.innerHTML = "Enter Here";
  cell2.innerHTML = "Enter Here";
  cell1.setAttribute("contenteditable","true");
  cell2.setAttribute("contenteditable","true");
  console.log("add_row(): Added row.");
}


/* 
Function: Check Commit-id
Usage: Checks for valid commit ID
Returns: false if B-ID is null
*/

function check_commit_id(bid)
{
  var flag = 0;
  if(bid === null)
  {
    return false;
  }

  if(bid.length)
  {
    var res = bid.trim().split("-");
    if(bid.length==7 && bid.indexOf('-') > -1 && bid.startsWith("B"))
    {
      var nums = /^\d+$/.test(res[1]);
      if(nums)
      {
        flag = 1;
      }
    }
  }
  if(flag==0)
  {
    return false;
  }
  return true;
}

/*
Function: Commit MEssage Handler
Usage: Validates & make actual commit message to Flask Server
*/
function svn_commit()
{
  var bid = $('#B-id').val();
  var msg = $('#message-text').val();

  var check = check_commit_id(bid);
  if(check == false)
  {
    alert("Invalid message: Usage< B-XXXXX (X:-integer-ID)>");
    return false;
  }

  if(msg.length < 3)
  {
    alert("invalid Message Length: should be atleast 4 characters long.");
    return false;
  }


  var env_name = document.getElementById('myenv_id').innerHTML;
  console.log("SVN_Commit: environment Name: "+ env_name);
  var myMesg = bid + " - " + msg;

  console.log("svn_commit: Commit message is:" +myMesg);

  // Ajax Call 
  $.ajax({
    type: 'POST',
    contentType: 'application/json;charset=UTF-8',
    data: JSON.stringify({msg_data:myMesg, my_env:env_name}),
    url: '/settings/my_commit',
    success: function (data)
    {
      console.log("svn_commit: Success. data received:" +data.SVN_MSG);
      create_table_rows(data);
    }
  });

  return true;
}

/*
Function: Check for XML input Errors
Usage: checks any XML errors.
*/
function xml_error_check(tag_name, value)
{
  var check = true;

  var xmlString = "<"+tag_name+">"+value+"</"+tag_name+">";
  var oParser = new DOMParser();
  var oDOM = oParser.parseFromString(xmlString, "text/xml");

  console.log(oDOM.documentElement.nodeName);
  if(oDOM.documentElement.nodeName == "parseerror")
  {
    check = false;
    console.log("FOund XML Error.");
  }
  console.log("XML-ERROR-CHECK: Check Done.");
  return check;
}

/*
Function: Update Entries in XML File
Usage; when update clicked. gets all properties and sends update to
Flask.
*/
function update_xml()
{
  var myDict = [];

  var env_name = document.getElementById('myenv_id').innerHTML;
  var myTable = document.getElementById('dataTable');
  var numRows = myTable.rows.length;

  for (var i = 1; i < numRows; i++)
  {
    var cols = myTable.rows.item(i).cells;
    var numCols = cols.length;
    for(var j=1; j< numCols; j++)
    {
      var my_tag = cols.item(j).innerHTML.replace("<br>", '');
      var my_val = cols.item(j+1).innerHTML.replace("<br>", '');

      // Check for XML Errors
      var check = xml_error_check(my_tag, my_val);
      if(check == false)
      {
        var errorString = "<"+my_tag+">"+my_val+"</"+my_tag+">";
        alert("Invalid xml format string:"+errorString);
        return false;
      }

      var final_str = my_tag+"-->>"+my_val;
      myDict.push(final_str)
      j = 3;
    }
  }
  console.log("svn_update: Final List Updated:");

  // Send data with AJAX Call.
  $.ajax({
    type: 'POST',
    contentType: 'application/json;charset=UTF-8',
    data: JSON.stringify({data_dict:myDict,my_env:env_name}),
    url: '/settings/my_update',
    success: function (data)
    {
      console.log("AJAX: Update Method success. Data received..."+ data.hasOwnProperty('SVN_MSG'));
      create_table_rows(data);
    }
  });
}

/*
Function: Interactive Search for table
usage: search for anything in table
*/
function search_table()
{
  var searchText = document.getElementById('searchTerm').value;
  var targetTable = document.getElementById('dataTable');
  var targetTableColCount;

  for (var rowIndex = 0; rowIndex < targetTable.rows.length; rowIndex++)
  {
      var rowData = '';
      if (rowIndex == 0)
      {
        targetTableColCount = targetTable.rows.item(rowIndex).cells.length;
        continue; //do not execute further code for header row.
      }
      for (var colIndex = 0; colIndex < targetTableColCount; colIndex++)
      {
        var cellText = '';
        if (navigator.appName == 'Microsoft Internet Explorer')
          cellText = targetTable.rows.item(rowIndex).cells.item(colIndex).innerText;
        else
          cellText = targetTable.rows.item(rowIndex).cells.item(colIndex).textContent;

        rowData += cellText;
      }
      rowData = rowData.toLowerCase();
      searchText = searchText.toLowerCase();

      if (rowData.indexOf(searchText) == -1)
        targetTable.rows.item(rowIndex).style.display = 'none';
      else
        targetTable.rows.item(rowIndex).style.display = 'table-row';
  }
}

/*
Function: Remove Click returns here
usage: Removes selected rows from table.
*/
function delete_row()
{

  var table = document.getElementById("dataTable");
  var rowCount = table.rows.length;
  for(var i=1; i<rowCount; i++)
  {
    var row = table.rows[i];
    var chkbox = row.cells[0].getElementsByTagName('input')[0];
    if(null != chkbox && true == chkbox.checked)
    {
      table.deleteRow(i);
      rowCount--;
      i--;
    }
  }
}
