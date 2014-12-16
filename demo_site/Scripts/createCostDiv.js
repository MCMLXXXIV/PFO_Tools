var CostObject;

function CreateCostDiv(topLevelCostObject) {
    CostObject = topLevelCostObject;
    var retVal = $('<div></div>');
    $('<h3></h3>').text('Cost').appendTo(retVal);
    CreateCostList(CostObject).appendTo(retVal);
    return retVal;
}

function CreateCostList (costObj) {
    if (costObj == undefined) {
	return $('<span></span>').text("CreateCostList() with empty obj");
    }

    var list = $('<ul></ul>');
    for(var itemIdx = 0; itemIdx < costObj.length; itemIdx++) {
	var lineItem = $('<li></li>');
	var item = costObj[itemIdx];
	if (item.hasOwnProperty('Name')) {
	    var name = item.Name;
	    
	    $('<span></span>').text(name).appendTo(lineItem);
	    if (name != 'Item') {
		if (name == 'Time') {
		    $('<span></span>').text(": combined crafting/refining time of ").appendTo(lineItem);
		    $('<span></span>').text(PrettyPrintDuration(item.Quantity)).appendTo(lineItem);
		}
		else if (name == 'ExperiencePoint') {
		    $('<span></span>').text("s: ").appendTo(lineItem);
		    $('<span></span>').text(item.Quantity + ' points').appendTo(lineItem);
		    var prettyStringDur = PrettyPrintDuration(item.Quantity / 100 * 60 * 60);
		    $('<span></span>').text(', or ' + prettyStringDur + ' of xp').appendTo(lineItem);
		}
		else {
		    $('<span></span>').text(", rank/quantity: ").appendTo(lineItem);
		    $('<span></span>').text(item.Quantity).appendTo(lineItem);
		}
	    }
	    if (item.hasOwnProperty('Detail')) {
		CreateCostList(item.Detail).appendTo(lineItem);
	    }
	    if (item.hasOwnProperty('Children')) {
		CreateCostList(item.Children).appendTo(lineItem);		
	    }
	} else {
	    $('<span></span>').text(item).appendTo(lineItem);
	}
	lineItem.appendTo(list);
    }

    return list;
}