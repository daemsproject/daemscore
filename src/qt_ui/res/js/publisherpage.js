function preparePkgPub() {
    $("#theText").bind('input propertychange', function () {
        $("#confirmpub").removeAttr('disabled');
    });
    $("#confirmpub").click(function () {
        var text = $("#theText").val();
        //console.log(text);
        var feerate = BrowserAPI.getFeeRate(0.15);
        console.log(feerate);
        var result = BrowserAPI.publishPackage(gParam.accountID, text, Number(feerate));
        CPage.showNotice(result);
    });
}


$(document).ready(function () {
    //doTranslate();

    $("#tpls").load("templates.html", function () {
        CUtil.initGParam();
        CPage.prepareHeader(true);
        $("#mainframe").html($("#maininput-tpl").html()).attr("id", "maininput");
        prepareSimplePub();
        prepareStdTpl();
        CPage.prepareNotice("publisher");
        CPage.updateBalance();
        CPage.updateCblc();
        CPage.registerNotifications();
    });
    doTranslate();
});




