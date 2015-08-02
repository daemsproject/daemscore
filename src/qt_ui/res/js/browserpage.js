var newDisp = []; // array to host block range shown on new page
var fllDisp = []; // follow page
var mypDisp = []; // mypage
var sldMax = 100;
var sld = [];
$(document).ready(function () {

    CBrowser.newAction();

    $("#refresh-btn").click(function () {
        CBrowser.refreshNew();
    });
    $("#refreshold-btn").click(function () {
        CBrowser.refreshOld();
    });
    $(".tabbar").children("li").children("a.ntcbtn").click(function () {
        CBrowser.switchTab($(this).attr("id"));
    });
    $("br-lang-btn").click(function () {
    });
    $(".linkspan").click(function () {
        CUtil.copyToClipboard($(this).find("a").html());
    });
    $(".ctt-link-btn").click(function () {
        var link = $(this).parent().parent().find(".linkspan").attr("clink");
        CUtil.copyToClipboard(link);
    });
//    $("#fullImage").click(function () {
//        $(this).html("");
//    });
    $(".column2").scroll(function () {
        
        if ($(".column2").scrollTop() + $(".column2").height() >= $(".column2")[0].scrollHeight) {
//            alert("btm");
            CBrowser.bottomAction();
        }
    });

    var imgs = CBrowser.getNewImages();
    CBrowser.addSlideImage(imgs);
    function resetC2() {
//        var c1w = $("#slider ul li").width() + $("body").width() * 0.1;
//        $(".column2").css({marginLeft: c1w});


        var c1w = $("body").width() - $(".column1").width() - 150;
        var height = $(window).height() -50;
        $(".column2").css({width: c1w});
        $(".column2").css({height: height});
    }

    $(window).resize(function () {
        resetC2();
    });

// slider code from http://codepen.io/zuraizm/pen/vGDHl
    var sliderInterval = 3000;
    var sliderAuto = setInterval(function () {
        moveRight();
    }, sliderInterval);

    var slideCount = $('#slider ul li').length;
    var slideWidth = $('#slider ul li').width();
    var slideHeight = $('#slider ul li').height();
    var sliderUlWidth = slideCount * slideWidth;
    $('#slider').css({width: slideWidth, height: slideHeight});
    $('#slider ul').css({width: sliderUlWidth, marginLeft: -slideWidth});
    $('#slider ul li:last-child').prependTo('#slider ul');
    $('#slider').mouseover(function () {
        clearInterval(sliderAuto);
    }).mouseleave(function () {
        sliderAuto = setInterval(function () {
            moveRight();
        }, sliderInterval);
    });
    function moveLeft() {
        $('#slider ul').animate({
            left: +slideWidth
        }, 200, function () {
            $('#slider ul li:last-child').prependTo('#slider ul');
            $('#slider ul').css('left', '');
        });
    }
    function moveRight() {
        $('#slider ul').animate({
            left: -slideWidth
        }, 200, function () {
            $('#slider ul li:first-child').appendTo('#slider ul');
            $('#slider ul').css('left', '');
        });
    }
    $('a.control_prev').click(function () {
        moveLeft();
    });
    $('a.control_next').click(function () {
        moveRight();
    });

// copy from publisherpage.js
    var dropZone = document.getElementById('fileholder');
    var theTextVal = "Type some here or drag and drop a file / Click or drag the file into the area above";
    $("#theText").attr("placeholder", theTextVal);
//    $('#fileholder').click(function () {
//        CPublisher.handleFileInput('theFile');
//    })
    $('#cblc').html(BrowserAPI.getBlockCount());
    $('#browsefile').click(function () {
        CPublisher.handleFileInput('theFile');
    })
    $('#theText').click(function () {
//        console.log($(this).val());
        $(this).removeClass('lttext');
//        if ($(this).val() === theTextVal) {
//            $(this).val("").addClass("ltborder").removeClass("noborder");
        $('#pubbtnh').show();
//        }
//    }).blur(function () {
//        if ($(this).val() === "") {
//            $(this).addClass('lttext').addClass("noborder").removeClass("ltborder");
//        }
    });
    $('#addtag').click(function () {
//        alert('t');
        CPublisher.addTagField();
    });
    $('#addlink').click(function () {
        CPublisher.addLinkField();
        $(this).attr('disabled', true);
    });
    if (window.File && window.FileReader && window.FileList && window.Blob) {
        dropZone.addEventListener('dragover', CPublisher.handleDragOver, false);
        dropZone.addEventListener('drop', CPublisher.handleFileSelect, false);
    } else {
        alert("error handleDragOver");
    }
    $("#test-btn").click(function () {
//        var t = "test texttest";
//        CBrowser.showNotice(t, 100);
        var t = BrowserAPI.getHash('1234');
        console.log(t);
    });
    $("#theText").bind('input propertychange', function () {
        $("#confirmpub").removeAttr('disabled');
    });
    $("#input-link").bind('input propertychange', function () {
        $("#confirmpub").removeAttr('disabled');
    });

    $("#confirmpub").click(function () {
        var text = $("#theText").val();
        var haveFile = typeof bufferedFile.ctt !== 'undefined';
//        console.log(typeof bufferedFile.ctt);
//        console.log('text');
//        console.log(text);
        var linkstr = $("#input-link").val();
        var link = CLink.setString(linkstr, $("#input-link-name").val());
//        console.log(link);
        if (!link.isValid() && !link.isEmpty()) {
            CBrowser.showNotice("Link is invalid");
            return;
        }
//        console.log(link);
//        console.log(link.isEmpty());
//        console.log(link.isValid());
        var ctt;
        if (text === "") {
            if (!haveFile) {
                if (link.isEmpty()) {
                    CBrowser.showNotice("Nothing to send");
                    console.log("err --nothing to send");
                    return;
                } else {
                    ctt = CPublisher.createLinkContent(link);
                }
            } else
                ctt = bufferedFile.ctt;

        } else {
            if (haveFile) {
                console.log("bf.ctt");
                console.log(bufferedFile.ctt);
                ctt = CPublisher.addTextToContent(bufferedFile.ctt, text);
                if (!link.isEmpty())
                    ctt = CPublisher.addLinkToContent(ctt, link);
            } else {
                ctt = CPublisher.createTextContent(text);
                if (!link.isEmpty())
                    ctt = CPublisher.addLinkToContent(ctt, link);
            }
        }
//        console.log(ctt);
        CPublisher.handleContent(ctt);
    });
    doTranslate();
    $('#langmenu li a').click(function () {
        langCodeF = $(this).attr("tr");
        console.log(langCodeF);
        langCode = langCodeF.substring(0, 2);
        doTranslate();
    });
    $('#validtil').click(function () {
        console.log('vt');
        CPublisher.addValidtilField();
        $(this).attr('disabled', true);
        $("#validtil").datepicker();
    });
    resetC2(); // this line need to stay at the bottom
});