var accountID;
$(document).ready(function () {
    accountID = BrowserAPI.getAccountID(); 
    $("#theText").bind('input propertychange', function () {
        $("#confirmpub").removeAttr('disabled');
    });
    $("#confirmpub").click(function () {
        var text = $("#theText").val();
        console.log(text);
        var result=BrowserAPI.publishPackage(accountID,text);
        CBrowser.showNotice(result);
            
    });
});