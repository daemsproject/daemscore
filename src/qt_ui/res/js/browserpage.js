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
    $(window).scroll(function () {
        if ($(window).scrollTop() + $(window).height() == $(document).height()) {
            CBrowser.bottomAction();
        }
    });

    var imgs = CBrowser.getNewImages();
    CBrowser.addSlideImage(imgs);
    function resetC2() {
        var c1w = $("#slider ul li").width() + $("body").width() * 0.1;
        $(".column2").css({marginLeft: c1w});
    }
    resetC2();
    $(window).resize(function () {
        resetC2();
    });

// slider code from http://codepen.io/zuraizm/pen/vGDHl
    var sliderAuto = setInterval(function () {
        moveRight();
    }, 3000);

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
        }, 3000);
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


});
