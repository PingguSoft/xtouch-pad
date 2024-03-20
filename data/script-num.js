$(document).ready(function () {
    $('.num-spinner span').click(function () {
        var inp = $(this).parents('.num-spinner').find('input');
        var min = parseFloat(inp.attr('min'));
        var max = parseFloat(inp.attr('max'));
        var step = parseFloat(inp.attr('step'));

        if ($(this).hasClass('minus')) {
            var count = parseFloat(inp.val()) - step;
            count = Math.max(min, count);
            count = Math.min(max, count);
            if (count <= min) {
                $(this).addClass('disabled');
            }
            else {
                $(this).removeClass('disabled');
            }
            if (count < max) {
                $(this).parents('.num-spinner').find(('.plus')).removeClass('disabled');
            }
            inp.val(count);
        }
        else {
            var count = parseFloat(inp.val()) + step
            count = Math.max(min, count);
            count = Math.min(max, count);

            if (count >= max) {
                $(this).addClass('disabled');
            } else {
                $(this).removeClass('disabled');
            }
            if (count > min) {
                $(this).parents('.num-spinner').find(('.minus')).removeClass('disabled');
            }
            inp.val(count);
        }

        inp.change();
        return false;
    });
});