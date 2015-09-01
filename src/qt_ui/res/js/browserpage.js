var newDisp = []; // array to host block range shown on new page
var gParam = {}; // global params
var sldMax = 100;
var sld = [];
var currentTab = "br-new-btn";
//var page = "browser";
var accountID;
var balance = {};
$(document).ready(function () {
    console.log(new Date());
    $("#tpls").load("templates.html", function () {

        var page = CBrowser.getPageName();
        console.log(page + "  " + new Date());
        switch (page) {
            case "homepage":
                prepareStdTpl();
                var id = CUtil.getGet("id");
                CBrowser.goToHomepage(id);
                CBrowser.regBottomAction();
//                
                resetC2();
                break;
            case "link":
                prepareStdTpl();
                var link = CUtil.getGet("link");
                var format = CUtil.getGet("format");
//                console.log(format);
                format = format ? Number(format) : 7;
                format = format > 7 ? 7 : format;
                CBrowser.goToLinkpage(link, format);
                resetC2();
                break;
            case "browser":
            default:
                $("#maininput").html($("#maininput-tpl").html());
                prepareSimplePub();
                prepareStdTpl();
//
//                var imgs = CBrowser.getNewImages();
////                console.log(imgs);
//                CBrowser.addSlideImage(imgs);
//                CBrowser.prepareSlider();
                CBrowser.regBottomAction();
                CBrowser.newAction();
                resetC2(); // this line need to stay at the bottom
                CBrowser.refreshImages(true);
                setInterval(function () {
                    var tab = CBrowser.getCurrentTab();
                    if (tab === "br-home-btn")
                        CBrowser.refreshImages();
                }, 10000);
                break;
        }
        CBrowser.prepareNotice(page);
        $("a").click(function () {
            CBrowser.regLink($(this));
        });
    });
    $("#test1-btn").click(function () {
//        var t = BrowserAPI.goToCustomPage('http://google.com');
//        console.log(t);
        var t = window.location.href;
        console.log(t);
        console.log(window.location.search);
        if (t.indexOf("?") < 0)
            window.location.assign(t + "/?id=K4HUMJNWO4DT2G6QPTXPIYZDEHC6QGL5VR77R7K75MPGFHHAS36QEY2XGCCF");
//        window.location.reload();
    });
    $("#test2-btn").click(function () {
        var t = window.location.href;
        console.log(t);
        if (t.indexOf("?") < 0)
            window.location.assign(t + "?link=fai:607.1");
    });
    $(".bclink").click(function () {
        regBclink($(this));
    });
    $("#navi-name").click(function () {
        CBrowser.toggleIdOpt($(this));
    });
    function regBclink(div) {
        console.log('reg');
        var linkstr = div.attr("href");
        console.log(div.attr("href"));
        var nctt = BrowserAPI.getContentByLink(linkstr);
        console.log(nctt);
        if (!CBrowser.replaceContent(nctt, CONTENT_TYPE_FEED, div.parent().parent().parent().parent()))
            CBrowser.showNotice("Invalid link");
        $(".bclink").click(function () {
            regBclink($(this));
        });
    }

    function resetC2() {
        if ($("#shdr").length > 0) {
            var c1w = $("body").width() - $(".column1").width() - 150;
            $(".column2").css({width: c1w});
        } else {
            var c1w = $(window).width() * 0.95;
            $(".column2").css({left: 0});
            $(".column2").css({width: c1w});
        }
        var height = $(window).height() - 50;
        $(".column2").css({height: height});
    }

    $(window).resize(function () {
        resetC2();
    });
    accountID = BrowserAPI.getAccountID();
    balance = BrowserAPI.getBalance(accountID, false).balance;
    var bl = CBrowser.getBalanceLevel(balance);
    $('#cblc').html(BrowserAPI.getBlockCount());
    $('#balance').html(balance.balance_total);
    $("#blclvl").html(CBrowser.getBalanceHtml(bl));
    doTranslate();
    $("#getnew-btn").click(function () {
        CBrowser.newAction();
        $(this).addClass("hide");
    });

    function registerNotifications() {
        var aa = function (a) {
            CBrowser.notifyBlock(a);
        };
        BrowserAPI.regNotifyBlocks(aa);
    }
    registerNotifications();
    resetC2();
});