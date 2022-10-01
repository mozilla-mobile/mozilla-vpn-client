let clicks = 0;

(function(api) {
switch (clicks) {
  case 0:
    api.addon.badge = 1;
    api.addon.setTitle('', '🚀 D0M 🚀');
    api.addon.setSubtitle('', 'H4ll0 W0rlD!');

    api.addon.composer.blocks.find(b => b.id === 'a')
        .setText('', 'Please! Don\'t click the button');
    api.addon.composer.blocks.find(b => b.id === 'b')
        .setText('', '-> this button <-');
    api.addon.composer.blocks.find(b => b.id === 'b').style = 0;
    ++clicks;
    break;

  case 1:
    api.addon.badge = 2;
    api.addon.setTitle('', '💥 D0M 💥');
    api.addon.setSubtitle('', '_H_4_l_l_0_ _W_0_r_l_D_!_');

    api.addon.composer.blocks.find(b => b.id === 'a')
        .setText('', 'Please! Read here!! Do Not Click The Button!');
    api.addon.composer.blocks.find(b => b.id === 'b')
        .setText('', '-> THIS BUTTON <-');
    api.addon.composer.blocks.find(b => b.id === 'b').style = 1;
    ++clicks;
    break;

  default:
    api.addon.badge = 4;
    api.addon.setTitle('', 'Release the HypnoDrones');
    api.addon.setSubtitle('', `🌊💥🚀🌊 Target: ${api.settings.userEmail} 💥🚀🌊💥🚀`);

    api.addon.composer.blocks.find(b => b.id === 'a')
        .setText('', 'OK, click them all!');
    for (let i = 0; i < 10; ++i) {
      const button = api.addon.composer.create(
          `b_${i}`, 'button',
          {'content': `Button ${i}`, 'javascript': 'hypnodrones.js'});
      api.addon.composer.append(button);
    }

    api.addon.composer.remove('b');
    break;
}
})
