var CBrowser = new function () {
    var CBrowser = this;
    this.showFullId = function (div, fullId) {
        div.find("a.text").html(fullId.substring(0, 25) + "...");
        div.find("li").find("a").css('display', 'inline-block');
    };
    this.hideFullId = function (div, fullId) {
        div.find("a.text").html(fullId.substring(0, 10) + "...");
        div.find("li").find("a").hide();
    };
    this.toggleFullId = function (div) {
        var fullId = div.find("a.text").attr("fullid");
        if (fullId.substring(0, 25) + "..." === div.find("a.text").html())
            this.hideFullId(div, fullId);
        else
            this.showFullId(div, fullId);
    };
    this.getImageFrJson = function (json) {
        try {
            var ctt = json.content[0];
            var k;
            for (k in ctt.content) {
                if (ctt.content[k].cc_name == "CC_FILE_CONTENT")
                    return ctt.content[k].content;
            }
        } catch (e) {
            console.log("getImageFrJson error");
        }

    };
    this.getImage = function (clink) {
        var cj = (BrowserAPI.getContentByLink(clink));
        r = this.getImageFrJson(cj);
        return this.createImgHtml(clink, r);

    };
    this.createImgHtml = function (clink, imgB64Data) {
        return '<a ><img id="' + clink + '" src="data:image/jpg;base64,' + imgB64Data + '" class="brimg"/></a>';
    };
    this.toggleCmt = function (div) {
        var s = div.find("a.shrt").hasClass("short");
        if (s) {
            div.find("li").find("a").show();
            div.find("a.shrt").removeClass("short");

        } else {
            div.find("li").find("a").hide();
            div.find("a.shrt").addClass("short");
        }
    };
    this.showFullImg = function (div) {
        $("#fullImage").html(div.html());
        $("#fullImage").find("img").removeClass("brimg");
    };
    this.shortenId = function (id) {
        return id.substr(0, 10) + "...";
    };
    this.addContent = function (clink, ctt) {
        var sdiv = $("#standard").clone(true, true);
        sdiv.removeAttr("id");
        sdiv.find(".id").find(".text").attr("fullid", ctt.poster[0]);
        sdiv.find(".id").find(".text").html(this.shortenId(ctt.poster[0]));
        sdiv.find(".ctt").html(this.createImgHtml(clink, this.getImageFrJson(ctt)));
        $("#mainframe").prepend(sdiv.children());
//        console.log(ctt);
//        console.log(sdiv);
    };
    this.refreshNew = function () {
        var clink = "ccc:11175.1";
        var ctt = BrowserAPI.getContentByLink(clink);
        this.addContent(clink, ctt);
//        this.refreshEvent();
    };

};

$(document).ready(function () {
    $("#refresh-btn").click(function () {
        CBrowser.refreshNew();
    });
    $(".id").find("a.text").click(function () {
        console.log('log');
        CBrowser.toggleFullId($(this).parent());
    });
    $(".id-follow-btn").click(function () {
        alert("To Do");
    });
    $(".id-share-btn").click(function () {
        alert("To Do");
    });
    $(".brctt").find("a.shrt").click(function () {
        CBrowser.toggleCmt($(this).parent());
    });
    $(".ctt").click(function () {
        CBrowser.showFullImg($(this));
    });

    $("#fullImage").click(function () {
        $(this).html("");
    });
    $("#test1").html(CBrowser.getImage("ccc:4310.1"));
    $("#test2").html(CBrowser.getImage("ccc:11175.1"));
    $("#test-btn").click(function(){
        console.log('test');
        console.log(BrowserAPI.getRecent());
    });

});