var BankObject;

function CreateBankDiv(topLevelBankObject) {
    BankObject = topLevelBankObject;
    var retVal = $('<div></div>');
    $('<h3></h3>').text('Final Stock').appendTo(retVal);
    CreateBankList(BankObject).appendTo(retVal);
    return retVal;
}

function CreateBankList (bankObj) {
    if (bankObj == undefined) {
	return $('<span></span>').text("CreateBankList() with empty obj");
    }

    var abilityScores = { 'Intelligence':10, 'Personality':10, 'Dexterity':10, 'Strength':10, 'Wisdom':10, 'Constitution':10 };

    var list = $('<ul></ul>');
    for (var bankIdx = 0; bankIdx < bankObj.length; bankIdx++) {
	var item = bankObj[bankIdx];
	
	// minor hack here until I have better handling of starting and ending supply
	// that is: only show ability scores if they are higher than 10
	if (abilityScores.hasOwnProperty(item.Name) && abilityScores[item.Name] >= item.Quantity) {
	    continue;
	}

	var lineItem = $('<li></li>');
	$('<span></span>').text(item.Name).appendTo(lineItem);
	$('<span></span>').text(", rank/quantity: ").appendTo(lineItem);
	$('<span></span>').text(item.Quantity).appendTo(lineItem);
	lineItem.appendTo(list);
    }
    return list;
}

