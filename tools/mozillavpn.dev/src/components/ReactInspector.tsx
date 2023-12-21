import { Components } from "@vpntools/inspector"; 

import * as React from 'react';
import {createComponent} from '@lit/react';


export const MyElementComponent = createComponent({
    tagName: 'live-view',
    elementClass: Components.LiveView,
    react: React,
    events: {
      onactivate: 'activate',
      onchange: 'change',
    },
  });
