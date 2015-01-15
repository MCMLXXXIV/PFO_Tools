var CostObject;

function CreateTopLevelRequirementsDiv(topLevelReqs, rankArg) {
    var rank = parseInt(rankArg);
    var rankedReqsSize = topLevelReqs.length;
    var retVal = $('<div></div>');
    if (rankedReqsSize < 1) {
	return retVal;
    }
    
    $('<h3></h3>').text('Top Level Requirements').appendTo(retVal);
    
    // some hacky stuff here.  I use a bit of algorithmic shorthand to handle requirements for + items
    // The way things are right now, the crafted items don't have a separate list of requirements for
    // + items, you just provide an average level of bonus in the mats and you get the + on the final
    // product.  I take a shortcut and assume that you need all +2 ingredients for a +2 item.  Trouble is
    // that I don't have any explicit way of identifying things that take ranked ingredients except this:
    // If it's an item and we are trying to make a ranked item and it doesn't have separate requirements
    // but only requirements for rank zero, then I rank up the requirements as I do the plan.
    
    // This function returns the top level requirements for an item.  That means that we need to add the
    // same algorithm here so that we don't list the top level requirements for a +3 wand as:
    //     Item.Lesser Vital Gem +0, quantity 1
    //     Item.Pine Baton +0, quantity 1
    //     Item.Crimson Crystal +0, quantity 2
    
    var overrideRank = false;
    var selectedReqs;
    if (rankedReqsSize < rank + 1) {
	overrideRank = true;
	selectedReqs = topLevelReqs[0];
    } else {
	selectedReqs = topLevelReqs[rank];
    }
    
    var list = $('<ul></ul>');
    for (var itemIdx = 0; itemIdx < selectedReqs.length; ++itemIdx) {
	var lineItem = $('<li></li>');
	var item = selectedReqs[itemIdx];
	var name = item.Name;
	var matches = /^([^.]+)\.?/.exec(name);
	var type = matches[1];
	var lineText = '';
	if (type == 'Time') {
	    lineText = name + ': ' + PrettyPrintDuration(item.Quantity);
	} else if (type == 'Feat') {
	    lineText = name + ' Rank ' + item.Rank;
	} else if (type == 'ExperiencePoint') {
	    lineText = name + ': ' + item.Quantity + ' points or ' + PrettyPrintDuration(item.Quantity * 3600 / 100) + ' of xp';
	} else if (type == 'AbilityScore') {
	    if (item.Quantity > 10) {
		lineText = name + ': ' + item.Quantity;
	    } else {
		continue;
	    }
	} else if (type == 'AchievementPoint') {
	    lineText = name + ': ' + item.Quantity + ' points';
	} else if (type == 'Item') {
	    var itemRank = item.Rank;
	    if (overrideRank) {
		itemRank = rank;
	    }
	    if (itemRank == 0) {
		lineText = name + ', quantity ' + item.Quantity;
	    } else {
		lineText = name + ' +' + itemRank + ', quantity ' + item.Quantity;
	    }
	} else {
	    // I don't think anything will fall into here - but if it does I should add another clause
	    lineText = name + ', Rank ' + item.Rank + ', Quantity ' + item.Quantity;
	}
	$('<span></span>').text(lineText).appendTo(lineItem);
	lineItem.appendTo(list);
    }
    
    list.appendTo(retVal);
    return retVal;
}
