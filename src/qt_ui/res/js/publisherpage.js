function preparePkgPub() {
    $("#theText").bind('input propertychange', function () {
        $("#confirmpub").removeAttr('disabled');
    });
    $("#confirmpub").click(function () {
        var text = $("#theText").val();
        var feerate = FAI_API.getFeeRate(0.15);
        var result = FAI_API.publishPackage(gParam.accountID, text, Number(feerate));
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




