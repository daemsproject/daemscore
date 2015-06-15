var newDisp = []; // array to host block range shown on new page
var fllDisp = []; // follow page
var mypDisp = []; // mypage

$(document).ready(function () {

    CBrowser.newAction();

    $("#refresh-btn").click(function () {
        CBrowser.refreshNew();
    });
    $("#refreshold-btn").click(function () {
        CBrowser.refreshOld();
    });
    $(".tabbar").children("li").children("a").click(function () {
        CBrowser.switchTab($(this).attr("id"));
    });
    $(".linkspan").click(function () {
        copyToClipboard($(this).find("a").html());
    });
    $(".ctt-link-btn").click(function () {
        var link = $(this).parent().parent().find(".linkspan").attr("clink");
        copyToClipboard(link);
    });
    $("#fullImage").click(function () {
        $(this).html("");
    });
//    $("#test-btn").click(function () {
//        var addrs = [BrowserAPI.getAccountID()];
//        var bh = 4231;
//        console.log(BrowserAPI.getContents(bh, 10, false, addrs));
//    });
    $(window).scroll(function () {
        if ($(window).scrollTop() + $(window).height() == $(document).height()) {
            CBrowser.bottomAction();
        }
    });
});
