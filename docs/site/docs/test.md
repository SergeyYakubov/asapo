import Tabs from '@theme/Tabs';
import TabItem from '@theme/TabItem';

@ASAPO_VERSION@

<Tabs
  defaultValue="js"
  values={[
    { label: 'JavaScript', value: 'js', },
    { label: 'JavaScript', value: 'js2', },
    { label: 'Python', value: 'py', },
    { label: 'Java', value: 'java', },
  ]
}>
<TabItem value="js">

```jsx {1} link="getting_started/build.sh" snippetTag="#snippet1" title="/src/components/HelloCodeTitle.js"
```  

</TabItem>

<TabItem value="js2">

```jsx {1} link="./assets/build.sh" snippetTag="#snippet2" title="/src/components/HelloCodeTitle.js"
```  

</TabItem>

<TabItem value="py">

```py
def hello_world():
  print 'Hello, world!'
```

</TabItem>
<TabItem value="java">

```java
class HelloWorld {
  public static void main(String args[]) {
    System.out.println("Hello, World");
  }
}
```

</TabItem>
</Tabs>

