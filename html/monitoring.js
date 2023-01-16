var selectorOptions = {
    buttons: [ {
        step: 'hours',
        stepmode: 'backward',
        count: 1,
        label: '1hr'
    }, {
        step: 'hour',
        stepmode: 'backward',
        count: 3,
        label: '3hr'
    }, {
        step: 'hour',
        stepmode: 'backward',
        count: 6,
        label: '6hr'
    }, {
        step: 'hour',
        stepmode: 'backward',
        count: 12,
        label: '12hr'
    }, {
        step: 'day',
        stepmode: 'backward',
        count: 1,
        label: '1d'
    }, {
        step: 'day',
        stepmode: 'backward',
        count: 3,
        label: '3d'
    }, {
        step: 'week',
        stepmode: 'backward',
        count: 1,
        label: '1w'
    }, {
        step: 'week',
        stepmode: 'backward',
        count: 2,
        label: '2w'
    }, {
        step: 'month',
        stepmode: 'backward',
        count: 1,
        label: '1m'
    }, {
        step: 'month',
        stepmode: 'backward',
        count: 6,
        label: '6m'
    }, {
        step: 'year',
        stepmode: 'todate',
        count: 1,
        label: 'YTD'
    }, {
        step: 'year',
        stepmode: 'backward',
        count: 1,
        label: '1y'
    }, {
        step: 'all',
    }],
};


    
function gettable(command){
    
    return new Promise(function(resolve, reject){
	var xhr = new XMLHttpRequest();
	
	var url = "./cgi-bin/bashtest.cgi";
	
	var user ="root";
	var db="daq";
    
	// Set the request method to POST
	xhr.open("POST", url);
	
	// Set the request header to indicate that the request body contains form data
	xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
	
	
	var dataString = "user=" + user + "&db=" + db + "&command=" + command;
	
	
	// Send the request
	xhr.send(dataString);
	
	xhr.onreadystatechange = function() {
	    if (this.readyState == 4 && this.status == 200) {
		
		resolve(xhr.responseText);
	    }	       
//	    else reject(new Error('error loading'));
	}
    });
    
}


function updatedropdown(){

    //var xhr = new XMLHttpRequest();
    
    //var url = "./cgi-bin/bashtest.cgi";
    
    //var user ="root";
    //var db="daq";
    var command="SELECT distinct(name) from monitoring"
    
    // Set the request method to POST
    //xhr.open("POST", url);
    
    // Set the request header to indicate that the request body contains form data
    //xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
    
    
    //var dataString = "user=" + user + "&db=" + db + "&command=" + command;
    
    
    // Send the request
    //xhr.send(dataString);
    
    
    
    //xhr.onreadystatechange = function() {
    //	if (this.readyState == 4 && this.status == 200) {
    
    gettable(command).then(function(result){
	var tmp = document.getElementById("output")
	tmp.innerHTML = result;
	var table = document.getElementById("table");
	var select = document.getElementById("tableselect");
	
	for( var i=1; i < table.rows.length; i++){
	    
	    select.options.add(new Option( table.rows[i].innerText, table.rows[i].innerText));
	    
	}
	tmp.innerHTML = "";
	select.dispatchEvent(new Event("change"));
    });	    
    
}
		   
		   


updatedropdown();


// Select the select box
var select = document.querySelector('select');

select.addEventListener('change', function() {
    // Get the selected option
    var selectedOption = this.options[this.selectedIndex];
    var command = "select '*' from monitoring where name=\""+ selectedOption.value + "\"";
    
    var output= document.getElementById("output");
    gettable(command).then(function(result){

	output.innerHTML=result;
	var table = document.getElementById("table");
	table.style.display = "none";
	var xdata= new Map();
	var ydata= new Map();

	for( var i=1; i< table.rows.length; i++){
	    
	    var jsondata = JSON.parse(table.rows[i].cells[2].innerText);

	    for (let key in jsondata) {
		
		if( i == 1 ){
		    xdata.set(key,[table.rows[i].cells[0].innerText]);
		    ydata.set(key,[jsondata[key]]);
		}
		else{
		    xdata.get(key).push(table.rows[i].cells[0].innerText);
		    ydata.get(key).push(jsondata[key]);

		}
	    }
	}
	var data = [];
	for(let [key, value] of xdata){
	    
	    data.push({
		name: selectedOption.value + ":" +key,
		mode: 'lines',
		x: value,
		y: ydata.get(key)
	    });
	    
	}
	
	var layout = {
            title: 'Monitor Time series with range slider and selectors',          
            xaxis: {
		rangeselector: selectorOptions,
		rangeslider: {}
            }
	};

	var graphDiv = document.getElementById("graph"); 
	while(!document.getElementById("same").checked && graphDiv.data != undefined && graphDiv.data.length >0)
	{
	    Plotly.deleteTraces(graphDiv, 0);
	 //   Plotly.deleteTraces(graphDiv, [0]);
	}    	
	//Plotly.deleteTraces('graph', 0);
	Plotly.plot(graphDiv, data, layout);	    
	
    });
    // Perform an action with the selected option
    //console.log('You selected: ' + selectedOption.value);
    
});
