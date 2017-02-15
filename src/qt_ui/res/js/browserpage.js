var newDisp = []; // array to host block range shown on new page
var sldMax = 100;
var sld = [];
var currentTab = "br-new-btn";
var currentPage;
var gParam;
$(document).ready(function () {
    $("#tpls").load("templates.html", function () {
        CUtil.initGParam();
        if(FAI_API.getBlockCount()<26000)
            FAI_API.goToCustomPage("fai:help/?init=true");
        var page = CBrowser.getPageName();
        switch (page) {
            case "homepage":
                prepareStdTpl();
                var id = CUtil.getGet("id");
                CBrowser.goToHomepage(id);
                CBrowser.regBottomAction();
                resetC2();
                break;
            case "link":
                prepareStdTpl();
                var link = CUtil.getGet("link");
                console.log(link);
                if (CUtil.isLinkBlockChain(link)) {
                    var format = CUtil.getGet("format");
                    format = format ? Number(format) : 7;
                    format = format > 7 ? 7 : format;
                    CBrowser.goToLinkpage(link, format);
                    resetC2();
                } else {
                    console.log(link);
                    FAI_API.goToCustomPage(link);
                }
                break;
            case "share":
            case "cmt":
            case "browser":
            default:
                CPage.prepareHeader(true);
                CPage.updateBalance();
                $("#maininput").html($("#maininput-tpl").html());
                prepareSimplePub();
                prepareStdTpl();
                CBrowser.newAction();
                resetC2(); // this line need to stay at the bottom
                CBrowser.refreshImages(true);
                setInterval(function () {
                    var tab = CBrowser.getCurrentTab();
                    if (tab === "br-new-btn")
                        CBrowser.refreshImages();
                }, 10000);
                CBrowser.regBottomAction();
                if (page === "share") {
                    if (CUtil.getGet("link"))
                        CPublisher.shareLink(CUtil.getGet("link"));
                    else if (CUtil.getGet("id"))
                        CPublisher.shareId(CUtil.getGet("id"));
                } else if (page === "cmt") {
                    if (CUtil.getGet("link"))
                        CPublisher.commentLink(CUtil.getGet("link"), CUtil.getGet("id"));
                }
                break;
        }
        CPage.prepareNotice(page);
        $("a").click(function () {
            CBrowser.regLink($(this));
        });
    });
    $(".bclink").click(function () {
        regBclink($(this));
    });
    $("#navi-name").click(function () {
        CBrowser.toggleIdOpt($(this));
    });
    function regBclink(div) {
        var linkstr = div.attr("href");
        var nctt = FAI_API.getContentByLink(linkstr);
        if (!CBrowser.replaceContent(nctt, CONTENT_TYPE_FEED, div.parent().parent().parent().parent()))
            CPage.showNotice(TR("Invalid link"));
        $(".bclink").click(function () {
            regBclink($(this));
        });
    }
    $(window).resize(function () {
        resetC2();
    });
    doTranslate();
    $("#getnew-btn").click(function () {
        CBrowser.newAction();
        $(this).addClass("hide");
    });
    CPage.registerNotifications();
    resetC2();
});